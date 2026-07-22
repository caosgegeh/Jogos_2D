#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vector>
#include <ctime>

using namespace std;

// Uma coisa que eu fico confuso em meus códigos é se eu explico meu raciocinio, ou se ele já está óbvio o suficiente KKKKKKK
// pq os nomes das variaveis e funções sempre tento deixar auto explicativo

struct Controle {

    SDL_Event evento;
    bool rodando = true, vivo_player = true;
    SDL_Point local = {425, 275};
    int vel=8;
    int vem_boss = 0,vel_boss= 8;
    vector<bool> boss_vivo;
    vector<SDL_Rect> boss, balas;
};

struct Linha {
    int x1, y1, x2, y2;
};

struct Endereco_Player {
    int x=300, y=250, w=100, h=100;
};

class Config {
public:
    int Random() { // Nome em homenagem a biblioteca random do Python KKKK
        int numb = rand() % 660;
        return numb;
    }

    void Fundo(SDL_Renderer *renderer) {
        SDL_SetRenderDrawColor(renderer, 0, 50, 150,  255);
        SDL_RenderClear(renderer);
    }

    void Movimentos_do_jogador(const Uint8 *estado, SDL_Point &movimento_player, Endereco_Player &local_player) {
        Controle c;
        if (estado[SDL_SCANCODE_W]) { 
            movimento_player.y-= c.vel;
            local_player.y -= c.vel;
        }
        if (estado[SDL_SCANCODE_S]) {
            movimento_player.y+= c.vel;
            local_player.y += c.vel;
        }
        if (estado[SDL_SCANCODE_A]) {
            movimento_player.x-= c.vel;
            local_player.x -= c.vel; 
        }
        if (estado[SDL_SCANCODE_D]) {
            movimento_player.x+= c.vel;
            local_player.x += c.vel;
        }
    }

    void Renderizar_jogador(int x, int y, SDL_Renderer *renderer, bool vivo_player) {
        vector<Linha> player = {
            // Linha reta: "|"
            // Altura: 100
            // Largura: 0
            {300+x, 250+y, 300+x, 350+y},

            // Linha decrescente: "\"
            // Altura: 50
            // Largura: 100
            {300+x, 250+y, 450+x, 300+y},

            // Linha crescente: "/"
            // Altura: 50
            // Largura: 100
            {300+x, 350+y, 450+x, 300+y}

        };
        if (vivo_player) {
            // Renderizando o jogador
            for (int i = 0; i < player.size(); i++) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 190, 255);
                SDL_RenderDrawLine(renderer, player[i].x1, player[i].y1, player[i].x2, player[i].y2);
            }

            // Ideia intuitiva de pintar jogador
            // Minha logica foi de:
            //  E se eu pegar a linha de cima, e arastar até em baixo, mas só o canto esquerdo
            //  Nessa situação eu só preciso usar um for, que vai até a diferença dos cantos direitos da linha crescente - decrescente
            //  E meu pensamento estava parcialmente errado, a dirença entre entre esses dois pontos, permite só eu pinta a metade do player
            //  Ficando com a outra metade transparente, mas se só pinta até a metade, eu pensei em multiplicar essa diferença chamada de "limite" por 2
            //  E funcionou, mas confesso que não entendo pq a diferença de dois eixos não tinha sido suficiente, sendo que é literalmente a distancia entre eles

            int limite = player[1].y2 - player[1].y1;
            for (int i = 0; i < limite*2;i++) {
                SDL_RenderDrawLine(renderer, player[1].x1, player[1].y1+i, player[1].x2, player[1].y2); 
            }
        }
        
    }

    void Renderizar_boss(Controle &c, SDL_Renderer *renderer, int i) {
        SDL_SetRenderDrawColor(renderer, 0, 250, 0, 255);
        SDL_RenderFillRect(renderer, &c.boss[i]);
        c.boss[i].x-= c.vel_boss;
    }

    void Eliminar_boss(Controle &c, int i) {
        c.boss.erase(c.boss.begin() + i);
        c.boss_vivo.erase(c.boss_vivo.begin() + i);
    }

    bool Colisao_Player_e_Boss(Endereco_Player A, SDL_Rect B) {
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


};

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    Config a1;
    Endereco_Player local_player;
    Controle c;
    const Uint8 *estado = SDL_GetKeyboardState(NULL);
    SDL_Point movimento_player = {0, 0};

    srand(time(NULL));

    SDL_Window *window = SDL_CreateWindow(
        "Projeto Delta",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        880, 660, 0
    );
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    while (c.rodando) {
        while (SDL_PollEvent(&c.evento)) {
            if (c.evento.type == SDL_QUIT) {
                c.rodando = false;
            }
            if (c.evento.key.keysym.sym == SDLK_e && c.vivo_player) {
                c.balas.push_back({local_player.x, local_player.y+(local_player.h/2), 15, 6});
            }
        }

        a1.Movimentos_do_jogador(estado, movimento_player, local_player);
        
        a1.Fundo(renderer);

        a1.Renderizar_jogador(movimento_player.x, movimento_player.y, renderer, c.vivo_player);

        // Um intervalo de tempo para criarmos um Boss
        // Após reiniciamos a contagem do intervalo
        if (c.vem_boss > 75) {
            c.boss.push_back({900, a1.Random(), 50, 50});
            c.boss_vivo.push_back(true);
            c.vem_boss = 0;
        }


        for (int i = 0; i < c.boss.size(); i++) {

            if (c.boss_vivo[i]) {
                a1.Renderizar_boss(c, renderer, i);
            }

            if (c.boss[i].x+c.boss[i].w < 0) {
                a1.Eliminar_boss(c, i);
                c.vel_boss++;
                
            }
            if (a1.Colisao_Player_e_Boss(local_player, c.boss[i])) {
                c.vivo_player = false;
            }
        }
        for (int j = 0; j < c.balas.size(); j++) {
            c.balas[j].x+= c.vel*2;

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &c.balas[j]);

            for (int i =0; i<c.boss.size(); i++) {
                if (a1.Colisao(c.balas[j], c.boss[i])) {
                    a1.Eliminar_boss(c, i);
                    if (c.vel_boss > 1) {
                        c.vel_boss--;
                    }
                }
            }
            
        }

        
        c.vem_boss++;
        SDL_Delay(12);
        SDL_RenderPresent(renderer);

    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}



