#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdbool.h>


void Seguir(int *Alocal, int *Blocal, int Atamanho, int Btamanho) {

    int AcentroY = *Alocal + (Atamanho/2);
    int BcentroY = *Blocal + (Btamanho/2);
    int vel = 1;

    if (AcentroY > BcentroY) {
        *Blocal +=vel;
    }
    if (AcentroY < BcentroY) {
        *Blocal -=vel;
    }
}

bool Colisao(SDL_Rect A, SDL_Rect B) {
    if (
        A.x + A.w > B.x &&
        B.x + B.w > A.x &&
        A.y + A.h > B.y &&
        B.y + B.h > A.y
    ) {
        return true;
    }
    return false;
}

//  função criada para renderizar o personagem conforme a vida dele
// Originalmente feita pro Boss, mas tambem vou utilizei pro player
void Boss_funcao(SDL_Rect *boss, int vida_boss, SDL_Renderer *renderer) {
    if (vida_boss > 0) {
        switch (vida_boss) {
            case 4: SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); break;
            case 3: SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); break;
            case 2: SDL_SetRenderDrawColor(renderer, 255, 122, 0, 255); break;
            case 1: SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); break;
        }
        SDL_RenderFillRect(renderer, boss);
    }
}

bool Boss_atire(SDL_Rect A, SDL_Rect B) {
    if (A.y + A.h > B.y  && B.y + B.h > A.y) {
        return true;
    }
    return false;
}

//  Função criada em prol de auxiliar a compreensão dos comandos
void Eliminar_bala(bool *boss_tiro, int *mover_bala_boss) {
    *boss_tiro = false;
    *mover_bala_boss = 0;
}

int main() {
    // Ferramentas da Janela
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
        "Protocolo Alfa",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600, 0
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Variaveis ao jogo

    SDL_Event evento;
    SDL_Point local[3] = {
        // Player
        {70, 200}, 
        // Boss
        {715, 230}, 
        // Parede
        {350, 300}
    }, tamanho = {25, 125}, tiros[100];


    int vel=5, vida_boss = 4,vida_player = 4, pause = 0, mover_bala_boss = 0, imortalidade = 0;
    bool atirou[100], rodando = true, boss_tiro = false;

    for (int i=0;i<100;i++) {
        atirou[i] = false;
    }

    const Uint8 *estado = SDL_GetKeyboardState(NULL);

    // Loop principal
    while (rodando) {
        if(SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) {
                rodando = false;
            }
            if (evento.key.keysym.sym == SDLK_f && evento.type == SDL_KEYDOWN) {
                for (int i=0; i < 100; i++) {
                    atirou[i] = true;
                    tiros[i].x = local[0].x;
                    tiros[i].y = local[0].y + (tamanho.y/2);
                }
            }
        }

        if (estado[SDL_SCANCODE_W]) { local[0].y -= vel; }
        if (estado[SDL_SCANCODE_S]) { local[0].y += vel; }

        
        SDL_SetRenderDrawColor(renderer, 150, 0, 150,  255);
        SDL_RenderClear(renderer);



        // Player
        SDL_Rect player = {local[0].x, local[0].y, tamanho.x, tamanho.y};
        Boss_funcao(&player, vida_player, renderer);

        // Boss
        SDL_Rect boss = {local[1].x, local[1].y, tamanho.x, tamanho.y};
        Boss_funcao(&boss, vida_boss, renderer);

        Seguir(&player.y, &local[1].y, player.h, tamanho.y);

        // Parede
        SDL_SetRenderDrawColor(renderer, 0, 50, 255, 255);
        SDL_Rect parede = {local[2].x, local[2].y, 50, 250};
        SDL_RenderFillRect(renderer, &parede);

        // Movimentações da parede
        // Tentei usar só a variavel parede.y, mas por algum motivo desconhecido não funcionava
        // Então substituir pela variavel local[2].y, que significa a mesma coisa, só que de forma menos direta.
        local[2].y -=3;
        if (local[2].y+parede.h <= 0) {
            local[2].y = 800;
        }
        
        for (int i=0;i<100;i++) {
            if(atirou[i]) {
                tiros[i].x +=15;

                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect olha_o_tiro = {tiros[i].x, tiros[i].y, 15, 4};
                SDL_RenderFillRect(renderer, &olha_o_tiro);

                if (tiros[i].x > 800 || Colisao(parede, olha_o_tiro)) {
                    atirou[i] = false;
                }

                if (Colisao(olha_o_tiro, boss) && pause <= 0) {
                    vida_boss--;
                    pause = 10;
                    atirou[i] = false;
                }

            }
        }

        if (Boss_atire(boss, player) && pause <=0 && vida_boss > 0) {
            boss_tiro = true;
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect boss_bala = {local[1].x+mover_bala_boss, local[1].y+(tamanho.y/2), 15, 4};


        if (boss_tiro) {
            mover_bala_boss -= 10;
            SDL_RenderFillRect(renderer, &boss_bala);
        }

        if (boss_bala.x < 0 || Colisao(parede, boss_bala)) {
            Eliminar_bala(&boss_tiro, &mover_bala_boss);
        }

        if (Colisao(boss_bala, player) && imortalidade <= 0) {
            vida_player--;
            imortalidade = 15;
        }

        
        pause--;
        imortalidade--;
        SDL_Delay(12);
        SDL_RenderPresent(renderer);

    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}



//  SDL_Rect rect;
//  SDL_RenderDrawRect(renderer, &rect); vazio
//  SDL_RenderFillRect(renderer, &rect); preenchido
//  SDL_RenderDrawLine(renderer, x1, y1, x2, y2);


//  Compilação
//  clang A1.c -o jogo_teste $(pkg-config --libs sdl2)
