#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdbool.h>


//  coordenadas
struct Eixo {
    int x;
    int y;
};

struct Ferramentas {
    // 'vel' = velocidade
    // 'imortalidade' tempo de imunidade do player após ser baleado
    int pause, vel, imortalidade, vida_boss, vida_player, mover_bala_boss;
    bool rodando, boss_tiro;
};

struct Balas_Player {
    bool *atirou;
    // 'quant_tiros' quantidade de vetores do ponteiro de tiros
    // 'loop' variavel para loops que percorrem a quantidade de vetores de tiro
    int loop, quant_tiros;
    struct Eixo *tiros;
};

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
void Eliminar_bala(struct Ferramentas *f) {
    (*f).boss_tiro = false;
    (*f).mover_bala_boss = 0;
}

void Player_atire(struct Balas_Player *b, struct Eixo local, struct Eixo tamanho) {
    // Adionamos um tiro, e memorizarmos ele em 'loop'
    ++(*b).quant_tiros;
    (*b).loop = (*b).quant_tiros;

    // Atualizamos nossos vetores com a nova quantidade de tiros
    (*b).tiros = realloc((*b).tiros, sizeof(SDL_Point)*(*b).quant_tiros);
    (*b).atirou = realloc((*b).atirou, sizeof(bool)*(*b).quant_tiros);

    // Ativamos os novos vetores adionados
    (*b).atirou[(*b).quant_tiros-1] = true;
    (*b).tiros[(*b).quant_tiros-1].x = local.x;
    (*b).tiros[(*b).quant_tiros-1].y = local.y + (tamanho.y/2);
}



void Dano_ao_boss(struct Balas_Player *b, struct Ferramentas *f, int i) {
    (*f).vida_boss--;
    (*f).pause = 10;
    (*b).atirou[i] = false;
    --(*b).quant_tiros;
}

int main() {

    struct Balas_Player b = {
        .atirou = NULL,
        .loop = 0,
        .quant_tiros = 0
    };
    struct Eixo local[3] = {
        {
            // Player 
            .x = 70,
            .y = 200
        },
        {
            // Boss
            .x =715,
            .y =230
        },
        {
            // Parede
            .x =350,
            .y =300
        }
    }, tamanho = {.x= 25, .y=125};

    struct Ferramentas f = {
        .vel =5,
        .imortalidade = 0,
        .pause = 0,
        .vida_boss =4,
        .vida_player =4,
        .mover_bala_boss = 0,
        .rodando = true,
        .boss_tiro = false,
    };

    // Ferramentas da Janela
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
        "Protocolo Alfa",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600, 0
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Event evento;
    const Uint8 *estado = SDL_GetKeyboardState(NULL);

    // Loop principal
    while (f.rodando) {
        if(SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) {
                f.rodando = false;
            }
            if (evento.key.keysym.sym == SDLK_f && evento.type == SDL_KEYDOWN) {
                Player_atire(&b, local[0], tamanho);
            }
        }

        // Movimentações do Player
        if (estado[SDL_SCANCODE_W]) { local[0].y -= f.vel; }
        if (estado[SDL_SCANCODE_S]) { local[0].y += f.vel; }

        
        SDL_SetRenderDrawColor(renderer, 150, 0, 150,  255);
        SDL_RenderClear(renderer);



        // Player
        SDL_Rect player = {local[0].x, local[0].y, tamanho.x, tamanho.y};
        Boss_funcao(&player, f.vida_player, renderer);

        // Boss
        SDL_Rect boss = {local[1].x, local[1].y, tamanho.x, tamanho.y};
        Boss_funcao(&boss, f.vida_boss, renderer);

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
        for (int i=0;i<b.loop;i++) {
            if(b.atirou[i]) {

                b.tiros[i].x +=15;

                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect olha_o_tiro = {b.tiros[i].x, b.tiros[i].y, 15, 4};
                SDL_RenderFillRect(renderer, &olha_o_tiro);


                if (b.tiros[i].x > 800 || Colisao(parede, olha_o_tiro)) {
                    b.atirou[i] = false;
                    b.quant_tiros--;
                }

                if (Colisao(olha_o_tiro, boss) && f.pause <= 0) {
                    Dano_ao_boss(&b, &f, i);
                }

            }
        }

        if (Boss_atire(boss, player) && f.pause <=0 && f.vida_boss > 0) {
            f.boss_tiro = true;
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect boss_bala = {local[1].x+f.mover_bala_boss, local[1].y+(tamanho.y/2), 15, 4};


        if (f.boss_tiro) {
            f.mover_bala_boss -= 10;
            SDL_RenderFillRect(renderer, &boss_bala);
        }

        if (boss_bala.x < 0 || Colisao(parede, boss_bala)) {
            Eliminar_bala(&f);
        }

        if (Colisao(boss_bala, player) && f.imortalidade <= 0) {
            f.vida_player--;
            f.imortalidade = 15;
        }

        f.pause--;
        f.imortalidade--;

        SDL_Delay(12);
        SDL_RenderPresent(renderer);

    }
    free(b.tiros);
    free(b.atirou);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}