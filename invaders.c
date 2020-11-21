#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#define LINHAS_CREW 4
#define COLUNAS_CREW 7

#define DIST_CREW 30

#define FPS 60.0

#define SCREEN_W 960
#define SCREEN_H 720

const int GROUND_H = 100;

const int VEL_IMP = 2;

int facada = 0;
int score = 0;

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_BITMAP *icon = NULL;
ALLEGRO_BITMAP *fundo = NULL;
ALLEGRO_BITMAP *impostor = NULL;
ALLEGRO_BITMAP *idle_impostor = NULL;
ALLEGRO_BITMAP *crew = NULL;
ALLEGRO_BITMAP *faca = NULL;
ALLEGRO_FONT *fonte = NULL;
ALLEGRO_SAMPLE *som_morte = NULL;
ALLEGRO_SAMPLE *som_facada = NULL;
ALLEGRO_SAMPLE *som_start = NULL;
ALLEGRO_SAMPLE *vitoria = NULL;
ALLEGRO_AUDIO_STREAM *musica = NULL;


int inicializar() {
	//inicializa o Allegro
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return 0;
	}

	//inicializa o modulo que permite carregar imagens no jogo
	if(!al_init_image_addon()) {
		fprintf(stderr, "failed to initialize image module!\n");
		return 0;
	}

	//inicializa o módulo de fontes do Allegro
    al_init_font_addon();
	
	//inicializa o módulo ttf do Allegro
    if(!al_init_ttf_addon()) {
		fprintf(stderr, "failed to initialize ttf!\n");
        return 0;
    }

	//addon de audio
    if(!al_install_audio()) {
        fprintf(stderr, "failed to initialize audio!\n");
        return 0;
    }
 
    //addon que da suporte as extensoes de audio
    if(!al_init_acodec_addon()) {
        fprintf(stderr, "failed to initialize audio codec!\n");
        return 0;
    }
 
    //cria o mixer (e torna ele o mixer padrao), e adciona 5 samples de audio nele
    if (!al_reserve_samples(5)) {
        fprintf(stderr, "failed to reserve audio samples!\n");
        return 0;
    }

	//carrega o arquivo da fonte
	fonte = al_load_ttf_font("VCR_OSD_MONO.ttf", 40, 0);
    if (!fonte) {
        fprintf(stderr, "failed to load font!\n");
        return 0;
    }

    //cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		// al_destroy_timer(timer);
		return 0;
	}
	al_set_window_title(display, "Among Invaders");

	//cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return 0;
	}

	//cria a fila de eventos
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return 0;
	}

	//carrega o ícone
    icon = al_load_bitmap("icon.bmp");
    if (!icon){
        fprintf(stderr, "failed to load icon!\n");
		return 0;
    }
	al_set_display_icon(display, icon);

	//carrega o fundo
    fundo = al_load_bitmap("fundo.bmp");
    if (!fundo) {
        fprintf(stderr, "failed to load background!\n");
		return 0;
    }

	//carrega a folha de sprites do impostor
    impostor = al_load_bitmap("impostor.bmp");
    if (!impostor) {
        fprintf(stderr, "failed to load impostor!\n");
		return 0;
    }
    //usa a cor rosa como transparencia
    al_convert_mask_to_alpha(impostor,al_map_rgb(247,0,255));

	//carrega o sprite do impostor parado
    idle_impostor = al_load_bitmap("idle_impostor.bmp");
    if (!idle_impostor) {
        fprintf(stderr, "failed to load idle impostor!\n");
		return 0;
    }
    //usa a cor rosa como transparencia
    al_convert_mask_to_alpha(idle_impostor,al_map_rgb(247,0,255));

	//carrega a folha de sprites dos tripulantes
    crew = al_load_bitmap("crew.bmp");
    if (!crew) {
        fprintf(stderr, "failed to load crew!\n");
		return 0;
    }
    //usa a cor rosa como transparencia
    al_convert_mask_to_alpha(crew,al_map_rgb(247,0,255));

	//carrega a faca
    faca = al_load_bitmap("faca.bmp");
    if (!faca) {
        fprintf(stderr, "failed to load faca!\n");
		return 0;
    }
    //usa a cor rosa como transparencia
    al_convert_mask_to_alpha(faca,al_map_rgb(247,0,255));

	//carrega os samples
    som_morte = al_load_sample("som_morte.wav");
    if (!som_morte) {
        fprintf(stderr, "failed to load sample!\n");
		return 0;
    }
	som_facada = al_load_sample("som_facada.wav");
    if (!som_facada) {
        fprintf(stderr, "failed to load sample!\n");
		return 0;
    }
    som_start = al_load_sample("som_start.wav");
    if (!som_start) {
        fprintf(stderr, "failed to load sample!\n");
		return 0;
    }
	vitoria = al_load_sample("vitoria.wav");
    if (!vitoria) {
    	fprintf(stderr, "failed to load sample!\n");
		return 0;
    }

	//carrega o audio stream
    musica = al_load_audio_stream("musica.ogg", 4, 1024);
    if (!musica) {
    	fprintf(stderr, "failed to load audio stream!\n");
		return 0;
    }

	//liga o stream no mixer
    al_attach_audio_stream_to_mixer(musica, al_get_default_mixer());

    //define que o stream vai tocar no modo repeat
    al_set_audio_stream_playmode(musica, ALLEGRO_PLAYMODE_LOOP);

	//instala o teclado
	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return 0;
	}
	
	//instala o mouse
	if(!al_install_mouse()) {
		fprintf(stderr, "failed to initialize mouse!\n");
		return 0;
	}

	return 1;
}

typedef struct Impostor {
	//largura e altura de cada sprite dentro da folha
    int altura;
	int largura;
    //quantos sprites tem em cada linha da folha, e a atualmente mostrada
    int colunas_folha;
	int coluna_atual;
    //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
	int linhas_folha;
    int linha_atual;
    //posicoes X e Y da folha de sprites que serao mostradas na tela
    int regiao_x_folha;
	int regiao_y_folha;
    //posicao X Y da janela em que sera mostrado o sprite
    int pos_x;
	int pos_y;
    //velocidade X Y que o sprite ira se mover pela janela
    int vel_x;
	int vel_y;
} Impostor;

typedef struct Crew {
	//largura e altura de cada sprite dentro da folha
	int altura;
	int largura;
	//quantos sprites tem em cada linha da folha, e a atualmente mostrada
    int colunas_folha;
	int coluna_atual;
    //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
	int linhas_folha;
    int linha_atual;
	//posicoes X e Y da folha de sprites que serao mostradas na tela
    int regiao_x_folha;
	int regiao_y_folha;
    //posicao X Y da janela em que sera mostrado o sprite
    int pos_x;
	int pos_y;
    //velocidade X Y que o sprite ira se mover pela janela
    int vel_x;
	int vel_y;
} Crew;

typedef struct Faca {
	int pos_x;
	int pos_y;
	int altura;
	int largura;
} Faca;

void initImpostor(Impostor *imp) {
	imp->altura = 109;
	imp->largura = 77;
	imp->colunas_folha = 6;
	imp->coluna_atual = 0;
	imp->linhas_folha = 2;
	imp->linha_atual = 0;
	imp->regiao_x_folha = 0;
	imp->regiao_y_folha = 0;
	imp->pos_x = SCREEN_W / 2 - imp->largura;
	imp->pos_y = 530;
	imp->vel_x = 0;
	imp->vel_y = 0;
}

void initCrew(int i, int j, Crew *crewmate) {
	crewmate->largura = 60;
	crewmate->altura = 88;
	crewmate->colunas_folha = 6;
	crewmate->coluna_atual = 0;
	crewmate->linhas_folha = 2;
	crewmate->linha_atual = 0;
	crewmate->pos_x = j*crewmate->largura + j*DIST_CREW;
	crewmate->pos_y = i*crewmate->altura + i*DIST_CREW;
	crewmate->regiao_x_folha = 0;
	crewmate->regiao_y_folha = 0 + i*2*crewmate->altura;
	crewmate->vel_x = 1;
	crewmate->vel_y = crewmate->altura;
}

void initFaca(Faca *knife, Impostor imp) {
	knife->pos_x = imp.pos_x + imp.largura/2;
	knife->pos_y = imp.pos_y + imp.altura/2 - 10;
	knife->altura = 49;
	knife->largura = 22;
}

void desenhaImpostor(Impostor *imp) {
	if (imp->vel_x > 0) {
		//desenha sprite na posicao X Y da janela, a partir da regiao X Y da folha
		al_draw_bitmap_region(impostor,
							  imp->regiao_x_folha, imp->regiao_y_folha,
							  imp->largura, imp->altura,
							  imp->pos_x, imp->pos_y,
							  0);

	} else if (imp->vel_x < 0) {
		//desenha sprite, igual acima, com a excecao que desenha a largura negativa, ou seja, espelhado horizontalmente
		al_draw_scaled_bitmap(impostor,
							  imp->regiao_x_folha, imp->regiao_y_folha,
							  imp->largura, imp->altura,
							  imp->pos_x + imp->largura, imp->pos_y,
							  -imp->largura, imp->altura,
							  0);
	} else {
		al_draw_bitmap(idle_impostor,
					   imp->pos_x, imp->pos_y,0);
	}
}

void desenhaCrew(Crew *crewmate) {
	if (crewmate->vel_x > 0) {
		//desenha sprite na posicao X Y da janela, a partir da regiao X Y da folha
		al_draw_bitmap_region(crew,
							  crewmate->regiao_x_folha, crewmate->regiao_y_folha,
							  crewmate->largura, crewmate->altura,
							  crewmate->pos_x, crewmate->pos_y,
							  0);

	} else {
		//desenha sprite, igual acima, com a excecao que desenha a largura negativa, ou seja, espelhado horizontalmente
		al_draw_scaled_bitmap(crew,
							  crewmate->regiao_x_folha, crewmate->regiao_y_folha,
							  crewmate->largura, crewmate->altura,
							  crewmate->pos_x + crewmate->largura, crewmate->pos_y,
							  -crewmate->largura, crewmate->altura,
							  0);
	}
}

void desenhaFaca(Faca knife) {
	al_draw_bitmap(faca, 
				   knife.pos_x, knife.pos_y,
				   0);
}

int colisaoCrewSolo(Crew crewmate) {
	if (crewmate.pos_y + crewmate.altura >= SCREEN_H - GROUND_H){
		return 1;
	}
	return 0;
}

int colisaoCrewImpostor(Crew crewmate, Impostor imp) {
	if (imp.pos_y <= crewmate.pos_y + crewmate.altura - 5 && crewmate.pos_y - 5 <= imp.pos_y + imp.altura) {
		if (imp.pos_x <= crewmate.pos_x + crewmate.largura - 5 && crewmate.pos_x - 5 <= imp.pos_x + imp.largura) {
			return 1;
		}
	}
	return 0;
}

int colisaoCrewFaca(Crew crewmate, Faca knife) {
	if (crewmate.pos_y <= knife.pos_y + knife.altura && knife.pos_y <= crewmate.pos_y + crewmate.altura) {
		if (crewmate.pos_x <= knife.pos_x + knife.largura && knife.pos_x <= crewmate.pos_x + crewmate.largura) {
			return 1;
		}
	}
	return 0;
}

void deleteCrew(Crew *crewmate) {
	crewmate->largura = 0;
	crewmate->altura = 0;
	crewmate->colunas_folha = 6;
	crewmate->coluna_atual = 0;
	crewmate->linhas_folha = 2;
	crewmate->linha_atual = 0;
	crewmate->pos_x = 0;
	crewmate->pos_y = 0;
	crewmate->regiao_x_folha = 0;
	crewmate->regiao_y_folha = 0;
	crewmate->vel_x = 0;
	crewmate->vel_y = 0;
}

int novoRecorde(int score, int *recorde) {
	FILE *arq = fopen("recorde.txt", "r");
	*recorde = -1;
	if(arq != NULL) {
		fscanf(arq, "%d", recorde);
		fclose(arq);
	}
	if(*recorde < score ) {
		arq = fopen("recorde.txt", "w");
		fprintf(arq, "%d", score);
		fclose(arq);
		return 1;
	}
	return 0;
}

int main() {
	int i = 0, j = 0, m = 0, n = 0;


	//rotinas de inicialização
	if (!inicializar()){
        return -1;
    }

	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	//registra na fila os eventos de mouse (ex: clicar em um botao do mouse)
	al_register_event_source(event_queue, al_get_mouse_event_source());


    //quantos frames devem se passar para atualizar para o proximo sprite
    int frames_sprite = 6, cont_frames = 0;

	//criando impostor, crewmates e faca:
	Impostor imp;
	initImpostor(&imp);

	Crew crewmate[LINHAS_CREW][COLUNAS_CREW];
	for (i = 0; i < LINHAS_CREW; i++)	{
		for (j = 0; j < COLUNAS_CREW; j++) {
			initCrew(i, j, &crewmate[i][j]);
		}
	}

	Faca knife;
	initFaca(&knife, imp);

	//inicia o temporizador
	al_start_timer(timer);
	
	int waiting = 1;
	int playing = 0;
	int mostraRecorde = 0;

	while (waiting) {
		ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);
		
		if(ev.type == ALLEGRO_EVENT_TIMER) {
			//desenha o fundo na tela
            al_draw_bitmap_region(fundo,0,0,SCREEN_W,SCREEN_H,0,0,0);

			//desenha impostor, crewmates e faca
			for (i = 0; i < LINHAS_CREW; i++)	{
				for (j = 0; j < COLUNAS_CREW; j++) {
					desenhaCrew(&crewmate[i][j]);
				}
			}
			desenhaFaca(knife);
			desenhaImpostor(&imp);

			if( (int)(al_get_timer_count(timer)/FPS) % 2 == 0){
				al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W/2, SCREEN_H/2 + 125, ALLEGRO_ALIGN_CENTRE, "APERTE UMA TECLA PARA JOGAR");
			}


			//atualiza a tela (quando houver algo para mostrar)
			al_flip_display();
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			playing = 1;
			waiting = 0;
		}
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			waiting = 0;
			playing = 0;
		}
	}

	//loop principal do jogo
	al_play_sample(som_start, 1.0, 0.0,1.0,ALLEGRO_PLAYMODE_ONCE,NULL);
	while (playing) {
		ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {
			//animação do movimento do impostor e da crew
			//a cada disparo do timer, incrementa cont_frames
			cont_frames++;
			//se alcancou a quantidade de frames que precisa passar para mudar para o proximo sprite
			if (cont_frames >= frames_sprite) {
				//reseta cont_frames
				cont_frames=0;
				//incrementa a coluna atual, para mostrar o proximo sprite
				imp.coluna_atual++;
				//se coluna atual passou da ultima coluna
				if (imp.coluna_atual >= imp.colunas_folha){
					//volta pra coluna inicial
					imp.coluna_atual=0;
					//incrementa a linha, se passar da ultima, volta pra primeira
					imp.linha_atual = (imp.linha_atual+1) % imp.linhas_folha;
					//calcula a posicao Y da folha que sera mostrada
					imp.regiao_y_folha = imp.linha_atual * imp.altura;
				}
				//calcula a regiao X da folha que sera mostrada
				imp.regiao_x_folha = imp.coluna_atual * imp.largura;

				for (i = 0; i < LINHAS_CREW; i++)	{
					for (j = 0; j < COLUNAS_CREW; j++) {
						crewmate[i][j].coluna_atual++;
						if (crewmate[i][j].coluna_atual >= crewmate[i][j].colunas_folha){
							crewmate[i][j].coluna_atual=0;
							crewmate[i][j].linha_atual = (crewmate[i][j].linha_atual+1) % crewmate[i][j].linhas_folha;
							crewmate[i][j].regiao_y_folha = 0 + i*2*crewmate[i][j].altura;
						}
						crewmate[i][j].regiao_x_folha = crewmate[i][j].coluna_atual * crewmate[i][j].largura;
					}
				}
			}
 
            //atualiza as posicoes X Y do sprite de acordo com a velocidade, positiva ou negativa
            imp.pos_x += imp.vel_x;
            imp.pos_y += imp.vel_y;

            //se o impostor estiver perto da borda direita ou esquerda da tela para de mover
            if (imp.pos_x + imp.largura >= SCREEN_W) {
				imp.pos_x = SCREEN_W - imp.largura;
			} else if (imp.pos_x <= 0) {
                imp.pos_x = 0;
            }

			for (i = 0; i < LINHAS_CREW; i++)	{
				for (j = 0; j < COLUNAS_CREW; j++) {
					// se algum dos tripulantes encostar no canto da tela
					if (crewmate[i][j].pos_x + crewmate[i][j].largura + crewmate[i][j].vel_x > SCREEN_W || crewmate[i][j].pos_x + crewmate[i][j].vel_x < 0 ){
						// todos os tripulantes vão descer e inverter o sentido
						for (m = 0; m < LINHAS_CREW; m++)	{
							for (n = 0; n < COLUNAS_CREW; n++) {
								crewmate[m][n].pos_y += crewmate[m][n].vel_y;
								crewmate[m][n].vel_x *= -1;
							}
						}
					}
					crewmate[i][j].pos_x += crewmate[i][j].vel_x;
				}
			}

			//se a faca passar da tela é possível atirar de novo
            if (knife.pos_y + knife.altura <= 0) {
				facada = 0;
			}

			//desenha o fundo na tela
            al_draw_bitmap_region(fundo,0,0,SCREEN_W,SCREEN_H,0,0,0);

			//desenha a crew
			for (i = 0; i < LINHAS_CREW; i++)	{
				for (j = 0; j < COLUNAS_CREW; j++) {
					desenhaCrew(&crewmate[i][j]);
				}
			}

			//desenha a faca
			if (facada) {
				knife.pos_y -= 5;
			} else {
				initFaca(&knife, imp);
			}
			desenhaFaca(knife);
			
			//desenha o impostor
			desenhaImpostor(&imp);

			int colidiuFaca = 0, colidiuSolo = 0, colidiuImpostor = 0;

			for (i = 0; i < LINHAS_CREW; i++)	{
				for (j = 0; j < COLUNAS_CREW; j++) {
					colidiuFaca = colisaoCrewFaca(crewmate[i][j], knife);
					colidiuSolo = colisaoCrewSolo(crewmate[i][j]);
					colidiuImpostor = colisaoCrewImpostor(crewmate[i][j], imp);

					if (colidiuFaca) {
						deleteCrew(&crewmate[i][j]);
						score += 1;
						facada = 0;

						if (rand() % 2 == 0) {
							al_play_sample(som_morte, 1.0, 0.0,1.0,ALLEGRO_PLAYMODE_ONCE,NULL);
						} else {
							al_play_sample(som_facada, 1.0, 0.0,1.0,ALLEGRO_PLAYMODE_ONCE,NULL);
						}
						
					}
					if (colidiuSolo) {
						playing = !colidiuSolo;
						mostraRecorde = 1;
						al_rest(1);
						break;
					} else if (colidiuImpostor) {
						playing = !colidiuImpostor;
						mostraRecorde = 1;
						al_rest(1);
						break;
					}
				}
			}

			//desenha a pontuação
			al_draw_textf(fonte, al_map_rgb(255, 255, 255), 40, 40 , ALLEGRO_ALIGN_LEFT, "Score: %d", score);

			//atualiza a tela (quando houver algo para mostrar)
			al_flip_display();
		}
		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}
		//se o tipo de evento for um pressionar de uma tecla
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			switch (ev.keyboard.keycode) {
				case ALLEGRO_KEY_A:
					imp.vel_x = -VEL_IMP;
				break;

				case ALLEGRO_KEY_LEFT:
					imp.vel_x = -VEL_IMP;
				break;

				case ALLEGRO_KEY_D:
					imp.vel_x = VEL_IMP;
				break;
				
				case ALLEGRO_KEY_RIGHT:
					imp.vel_x = VEL_IMP;
				break;

				case ALLEGRO_KEY_SPACE:
					facada = 1;
				break;
			}
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			switch (ev.keyboard.keycode) {
				case ALLEGRO_KEY_A:
					imp.vel_x = 0;
				break;

				case ALLEGRO_KEY_LEFT:
					imp.vel_x = 0;
				break;

				case ALLEGRO_KEY_D:
					imp.vel_x = 0;
				break;

				case ALLEGRO_KEY_RIGHT:
					imp.vel_x = 0;
				break;
			}
		}
		
		if (score >= LINHAS_CREW * COLUNAS_CREW) {
			playing = 0;
			mostraRecorde = 1;
			al_rest(1);
		}
	}

	al_play_sample(vitoria, 1.0, 0.0,1.0,ALLEGRO_PLAYMODE_ONCE,NULL);
	
	while (mostraRecorde) {
		ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {
			int recorde;
			al_draw_bitmap_region(fundo,0,0,SCREEN_W,SCREEN_H,0,0,0);
			al_draw_textf(fonte, al_map_rgb(200, 0, 30), SCREEN_W/2, SCREEN_H/2 - 100, ALLEGRO_ALIGN_CENTRE, "Score: %d", score);
			if(novoRecorde(score, &recorde)) {
				al_draw_text(fonte, al_map_rgb(200, 20, 30), SCREEN_W/2, SCREEN_H/2, ALLEGRO_ALIGN_CENTRE, "NOVO RECORDE!");
			}
			else {
				al_draw_textf(fonte, al_map_rgb(0, 200, 30), SCREEN_W/2, SCREEN_H/2, ALLEGRO_ALIGN_CENTRE, "Recorde: %d", recorde);
			}
			//reinicializa a tela
			al_flip_display();
		}
		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			mostraRecorde = 0;
		}
	}


	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
	al_destroy_bitmap(icon);
	al_destroy_bitmap(fundo);
	al_destroy_bitmap(impostor);
	al_destroy_bitmap(idle_impostor);
	al_destroy_bitmap(crew);
	al_destroy_bitmap(faca);
	al_destroy_font(fonte);
	al_destroy_sample(som_morte);
	al_destroy_sample(som_facada);
	al_destroy_sample(som_start);
	al_destroy_sample(vitoria);
	al_destroy_audio_stream(musica);
	

    return 0;
}
