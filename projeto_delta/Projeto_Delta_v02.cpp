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
    double vel= 600.0;
    double vem_boss = 0.0, aceleracao_boss = 0;
    double vel_bala_player = vel*2;
    vector<bool> boss_vivo;
    vector<SDL_Rect> boss, balas;
};

struct Linha {
    double x1, y1, x2, y2;
};

struct Eixo {
    double x;
    double y;
};

struct Endereco_Player {
    double x=300, y=250, w=100, h=100;
};

class Config {
public:
    double Random() { // Nome em homenagem a biblioteca random do Python KKKK
        double numb = rand() % 660;
        return numb;
    }

    void Fundo(SDL_Renderer *renderer) {
        SDL_SetRenderDrawColor(renderer, 0, 50, 150,  255);
        SDL_RenderClear(renderer);
    }

    void Movimentos_do_jogador(const Uint8 *estado, Eixo &movimento_player, Endereco_Player &local_player, double delta_time) {
        Controle c;
        if (estado[SDL_SCANCODE_W]) { 
            movimento_player.y-= c.vel*delta_time;
            local_player.y -= c.vel*delta_time;
        }
        if (estado[SDL_SCANCODE_S]) {
            movimento_player.y+= c.vel*delta_time;
            local_player.y += c.vel*delta_time;
        }
        if (estado[SDL_SCANCODE_A]) {
            movimento_player.x-= c.vel*delta_time;
            local_player.x -= c.vel*delta_time; 
        }
        if (estado[SDL_SCANCODE_D]) {
            movimento_player.x+= c.vel*delta_time;
            local_player.x += c.vel*delta_time;
        }
    }

    void Renderizar_jogador(double x, double y, SDL_Renderer *renderer, bool vivo_player) {
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

    void Renderizar_boss(Controle &c, SDL_Renderer *renderer, int i, double delta_time, double aceleracao) {
        SDL_SetRenderDrawColor(renderer, 0, 250, 0, 255);
        SDL_RenderFillRect(renderer, &c.boss[i]);
        c.boss[i].x -= (aceleracao+10.0)*delta_time;
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
    Eixo movimento_player = {.x=0.0, .y=0.0};
    const Uint8 *estado = SDL_GetKeyboardState(NULL);
    double x1 =0.0, y1=0.0;
    Uint64 ultimo_time = SDL_GetPerformanceCounter();

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
                c.balas.push_back({(int)local_player.x, (int)(local_player.y+(local_player.h/2)), 15, 6});
            }
        }
        Uint64 atual_time = SDL_GetPerformanceCounter();
        double delta_time = (double)(atual_time - ultimo_time)/(double)SDL_GetPerformanceFrequency();

        a1.Movimentos_do_jogador(estado, movimento_player, local_player, delta_time);
        
        a1.Fundo(renderer);


        // Um intervalo de tempo para criarmos um Boss
        // Após reiniciamos a contagem do intervalo
        double segundos_de_intervalo = 0.75;
        if (c.vem_boss >= segundos_de_intervalo && c.vivo_player) {
            c.boss.push_back({900, (int)a1.Random(), 50, 50});
            c.boss_vivo.push_back(true);
            c.vem_boss = 0;
        }


        for (int i = 0; i < c.boss.size(); i++) {

            if (c.boss_vivo[i]) {
                a1.Renderizar_boss(c, renderer, i, delta_time, c.aceleracao_boss);
            }

            if (c.boss[i].x <= 0) {
                a1.Eliminar_boss(c, i);
                c.aceleracao_boss+=5;
                
            }
            if (a1.Colisao_Player_e_Boss(local_player, c.boss[i])) {
                c.vivo_player = false;
            }
        }
        for (int j = 0; j < c.balas.size(); j++) {
            c.balas[j].x+= c.vel_bala_player*delta_time;

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &c.balas[j]);

            for (int i =0; i<c.boss.size(); i++) {
                if (a1.Colisao(c.balas[j], c.boss[i])) {
                    a1.Eliminar_boss(c, i);
                    c.aceleracao_boss-=2;
                }
            }
            
        }

        a1.Renderizar_jogador(movimento_player.x, movimento_player.y, renderer, c.vivo_player);
        
        c.vem_boss+= delta_time;
        ultimo_time = atual_time;
        SDL_RenderPresent(renderer);

    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}



