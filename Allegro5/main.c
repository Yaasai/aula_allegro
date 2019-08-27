#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#define LARGURA_JANELA 1280
#define ALTURA_JANELA 720
#define MENU_NOVO_JOGO 0
#define MENU_SAIR 1
#define JOGO 3
#define NUMERO_PERGUNTAS 4
#define MAXIMO_CARACTERES 1000

typedef struct {
  char pergunta[MAXIMO_CARACTERES];
  char opcoes[4][MAXIMO_CARACTERES];
  int resposta;
} Pergunta;

static int estado_jogo = MENU_NOVO_JOGO;
static int resposta_selecionada;
static ALLEGRO_DISPLAY *janela; 
static ALLEGRO_EVENT_QUEUE *fila_de_eventos;
static ALLEGRO_FONT *fonte;
static Pergunta perguntas[NUMERO_PERGUNTAS];
static Pergunta pergunta_selecionada;

/**
 * Função que simplesmente printa uma mensagem de 
 * erro e concatena o nome do que falhou.
 * @Param nome: nome do que falhou na iniciação
 */
void erro_iniciacao(char *nome) {
  ALLEGRO_DISPLAY *janela_erro;
  char erro[200] = "Erro ao iniciar: ";
  strcat(erro, nome);
  strcat(erro, "\n");
  al_show_native_message_box(janela_erro,"Erro","Erro",erro,NULL,ALLEGRO_MESSAGEBOX_ERROR);
}

/**
 * Função que inicia todos os modulos necessários
 * do Allegro5.
 * @Return verdadeiro se todos modulos iniciaram ou falso se não.
 */
int iniciar_modulos() {

  if(!al_init()) {
    erro_iniciacao("Allegro");
    return 0;
  }

  if(!al_init_image_addon()) {
    erro_iniciacao("Image addon");
    return 0;
  }

  if(!al_init_font_addon()) {
    erro_iniciacao("Font addon");
    return 0;
  }

  if(!al_init_ttf_addon()) {
    erro_iniciacao("TTF addon");
    return 0;
  }
  
  al_install_keyboard();
  if(!al_is_keyboard_installed()) {
    erro_iniciacao("Teclado");
    return 0;
  }

  return 1;
}

/**
 * Função que desaloca objetos da memória.
 */
void limpar_memoria() {
  al_destroy_display(janela);
  al_destroy_event_queue(fila_de_eventos);
}


/**
 * Função que cria uma janela.
 * @Param x: tamanho horizontal da janela.
 * @Param y: tamanho vertical da janela.
 * @Return verdadeiro se foi iniciada corretamente ou falso se não.
 */
int iniciar_janela(int x, int y) {
  janela = al_create_display(x,y); 
  if(!janela) {
    erro_iniciacao("Janela");
    al_destroy_display(janela);
    return 0;
  }
  return 1;
}

/**
 * Função que cria uma janela.
 * @Return verdadeiro se foi iniciada corretamente ou falso se não.
 */
int iniciar_fila_eventos() {
  fila_de_eventos= al_create_event_queue();
  if(!fila_de_eventos) {
    erro_iniciacao("Fila de eventos"); 
    limpar_memoria(janela,fila_de_eventos);
    return 0;
  }
  al_register_event_source(fila_de_eventos,al_get_keyboard_event_source());
  al_register_event_source(fila_de_eventos,al_get_display_event_source(janela));
  return 1;
}

int iniciar_fonte() {
  fonte = al_load_ttf_font("./perguntas/fonte.ttf",36,0);
  if(!fonte) {
    erro_iniciacao("Fonte");
    return 0;
  }
  return 1;
}

Pergunta ler_pergunta(FILE *perguntas_txt) {
 
  Pergunta p;

  char pergunta[MAXIMO_CARACTERES];
  fgets(pergunta, MAXIMO_CARACTERES, perguntas_txt); 
  pergunta[strlen(pergunta)-2] = 0;
  strcpy(p.pergunta, pergunta);

  int i;
  for(i = 0; i < 4; i++) {
    char opcao[MAXIMO_CARACTERES];
    fgets(opcao, MAXIMO_CARACTERES, perguntas_txt); 
    opcao[strlen(opcao)-2] = 0;
    strcpy(p.opcoes[i], opcao); 
  }  
  
  char resposta[MAXIMO_CARACTERES];
  fgets(resposta, MAXIMO_CARACTERES, perguntas_txt); 
  resposta[strlen(resposta)-2] = 0;
  p.resposta = atoi(resposta); 

  return p;
  
}

int ler_perguntas(char *endereco) {

  FILE *perguntas_txt = fopen(endereco, "r");
  if(perguntas_txt == NULL) {
    erro_iniciacao(endereco);
    limpar_memoria(janela,fila_de_eventos);
    return 0;
  }

  int questao = 0;
  while ( questao < NUMERO_PERGUNTAS ) {
     perguntas[questao] = ler_pergunta(perguntas_txt);
     questao++;
  }

  fclose(perguntas_txt);
  
  return 1;
}

/**
 * Função que limpa a janela para determinada cor
 * @Param cor: cor para qual a janela vai ser limpada.
 */
void limpar_janela(ALLEGRO_COLOR cor) {
  al_clear_to_color(cor);
}

/**
 * Método que carrega, desenha e destroi uma imagem.
 * @Param endereco: endereço relativo da imagem.
 * @Param x: coordenadas x para desenhar a imagem.
 * @Param y: coordenadas y para desenhar a imagem.
 */
void desenhar_imagem(char *endereco, float x, float y) {
  ALLEGRO_BITMAP *imagem;
  imagem = al_load_bitmap(endereco);
  if(!imagem) {
    erro_iniciacao(endereco); 
    al_destroy_bitmap(imagem);
    return;
  }
  al_draw_bitmap(imagem,x,y,0);
  al_destroy_bitmap(imagem);
}

void atualizar_imagem(char *imagem_atual) {
  // IMAGENS
  char *menu_nj = "./imagens/menu_novo_jogo.png";
  char *menu_sair = "./imagens/menu_sair.png";
  char *lousa = "./imagens/vazia.png";
  
  switch(estado_jogo) {
    case MENU_NOVO_JOGO:
      strcpy(imagem_atual, menu_nj);
      break;
    case MENU_SAIR:
      strcpy(imagem_atual, menu_sair);
      break;
    case JOGO:
      strcpy(imagem_atual, lousa);
      break;
  }

  desenhar_imagem(imagem_atual,0,0);
}

void trocar_estado_menu() {
  
  if(estado_jogo == MENU_NOVO_JOGO) estado_jogo = MENU_SAIR;

  else if(estado_jogo == MENU_SAIR) estado_jogo = MENU_NOVO_JOGO;

}

int esta_em_jogo() {
  return estado_jogo == JOGO;
}

int esta_no_menu() {
  if(estado_jogo == MENU_NOVO_JOGO || estado_jogo == MENU_SAIR) return 1;
  return 0;
}

int pressionou_seta_cima(ALLEGRO_EVENT evento) {
  return evento.keyboard.keycode == ALLEGRO_KEY_UP;
}

int pressionou_seta_baixo(ALLEGRO_EVENT evento) {
  return evento.keyboard.keycode == ALLEGRO_KEY_DOWN;
}

int pressionou_enter(ALLEGRO_EVENT evento) {
  return evento.keyboard.keycode == ALLEGRO_KEY_ENTER;
}

int pressionou_uma_tecla(ALLEGRO_EVENT evento) {
  if(evento.type == ALLEGRO_EVENT_KEY_DOWN) return 1;
  return 0;
}

int pressionou_setas(ALLEGRO_EVENT evento) {
  if(evento.keyboard.keycode == ALLEGRO_KEY_LEFT || 
    evento.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
    return 1; 
  } 
  return 0;
}

Pergunta escolher_pergunta_aleatoriamente() {
  int questao = rand() % NUMERO_PERGUNTAS;  
  return perguntas[questao];
}

void desenhar_texto() {
  ALLEGRO_COLOR preto = al_map_rgb(0,0,0);
  ALLEGRO_COLOR vermelho = al_map_rgb(255,0,0);

  float x = 330, y = 330;
  al_draw_multiline_text(fonte,preto,x,200,820,36,ALLEGRO_ALIGN_LEFT,pergunta_selecionada.pergunta);

  int i;
  for( i = 0; i < 4; i++) {
    y += 30;
    if(i == resposta_selecionada) {
      al_draw_text(fonte,vermelho,x,y,ALLEGRO_ALIGN_LEFT,pergunta_selecionada.opcoes[i]);
      continue;
    }
    al_draw_text(fonte,preto,x,y,ALLEGRO_ALIGN_LEFT,pergunta_selecionada.opcoes[i]);
  }

}

void iniciar_novo_jogo() {
  estado_jogo = JOGO;
  pergunta_selecionada = escolher_pergunta_aleatoriamente();
  resposta_selecionada = 0;
}

void voltar_para_menu() {
  estado_jogo = MENU_NOVO_JOGO;
}

int resposta_certa() {
  return resposta_selecionada == pergunta_selecionada.resposta;
}

void verificar_resposta(char *imagem_atual) {
  char *acerto = "./imagens/acerto.png";
  char *erro = "./imagens/erro.png";
   
  if(resposta_certa()) {
    strcpy(imagem_atual, acerto);
  } else {
    strcpy(imagem_atual, erro);
  }

  desenhar_imagem(imagem_atual,0,0);
  al_flip_display();
  al_rest(2.0);
  al_flush_event_queue(fila_de_eventos);

}

void selecionar_resposta_baixo() {
  if(++resposta_selecionada > 3) {
    resposta_selecionada = 0;
  }
}

void selecionar_resposta_cima() {
  if(--resposta_selecionada < 0) {
    resposta_selecionada = 3;
  }
}

/**
 * Verifica se o jogador quer sair do jogo.
 * @Return 1 se o jogador quiser sair ou 0 se não. 
 */
int click_botao_sair(ALLEGRO_EVENT evento) {
  if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
    return 1;
  }
  if (pressionou_uma_tecla(evento)) {
    if(pressionou_enter(evento) && estado_jogo == MENU_SAIR) {
      return 1;
    }
  }
  return 0;
}

void atualizar_estado_jogo(ALLEGRO_EVENT evento, char *imagem_atual) {
  if(pressionou_uma_tecla(evento)) {
    if(esta_no_menu()) {
      if(pressionou_setas(evento)) {
        trocar_estado_menu();
        atualizar_imagem(imagem_atual);
        al_flip_display();
      } else if (pressionou_enter(evento) && estado_jogo == MENU_NOVO_JOGO) {
        iniciar_novo_jogo();
        atualizar_imagem(imagem_atual);
        desenhar_texto();
        al_flip_display();
      }
    } else if(esta_em_jogo()) {
      if(pressionou_enter(evento)) {
        verificar_resposta(imagem_atual);
        voltar_para_menu();
        atualizar_imagem(imagem_atual);
        al_flip_display();
      }
      else if(pressionou_seta_cima(evento)) {
        selecionar_resposta_cima(); 
        atualizar_imagem(imagem_atual);
        desenhar_texto();
        al_flip_display();
      }
      else if(pressionou_seta_baixo(evento)) {
        selecionar_resposta_baixo();
        atualizar_imagem(imagem_atual);
        desenhar_texto();
        al_flip_display(); 
      }
    }
  }
}

int main() {

  // CORES
  ALLEGRO_COLOR branco;
  ALLEGRO_COLOR preto;
  
  char imagem_atual[200];

  // Primeira coisa que você quer fazer é iniciar os modulos
  // do Allegro.
  if(!iniciar_modulos()) {
    return -1;
  }

  // Depois iniciamos as variáveis.
  if(!iniciar_janela(LARGURA_JANELA,ALTURA_JANELA)) {
    return -1;
  }
  if(!iniciar_fila_eventos()) {
    return -1;
  }
  if(!iniciar_fonte()) {
    return -1;
  }
  ler_perguntas("./perguntas/perguntas.txt");
  atualizar_imagem(imagem_atual);
  al_flip_display();
  branco = al_map_rgb(255,255,255);
  preto = al_map_rgb(0,0,0);

  // Loop do jogo
  int sair = 0;
  while(!sair) {

    ALLEGRO_EVENT evento;
    int evento_existe = al_wait_for_event_timed(fila_de_eventos,&evento,0.05);

    if(evento_existe) {
      sair = click_botao_sair(evento);
      atualizar_estado_jogo(evento, imagem_atual);
    }

  }

  limpar_memoria(janela, fila_de_eventos);

  return 0;  
}
