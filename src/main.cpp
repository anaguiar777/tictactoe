#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Estado do jogo
    char winner; // Vencedor do jogo

public:
    TicTacToe() {
        for (auto& row : board) {
            row.fill(' ');
        }
        current_player = 'X';
        game_over = false;
        winner = ' ';
        // Inicializar o tabuleiro e as variáveis do jogo
    }

    void display_board() {
        std::cout << "\n";
        for (int i = 0; i < 3; ++i) {
            std::cout << " ";
            for (int j = 0; j < 3; ++j) {
                std::cout << board[i][j];
                if (j < 2) std::cout << " | ";
            }
            std::cout << "\n";
            if (i < 2) std::cout << "---+---+---\n";
        }
        std::cout << "\n";
        // Exibir o tabuleiro no console
    }

    bool make_move(char player, int row, int col) {
    
        // Implementar a lógica para realizar uma jogada no tabuleiro
        // Utilizar mutex para controle de acesso
        // Utilizar variável de condição para alternância de turnos
        std::unique_lock<std::mutex> lock(board_mutex);

        // Espera até que seja a vez desse jogador ou o jogo acabe
        turn_cv.wait(lock, [&]() {
            return current_player == player || game_over;
        });

        // Se o jogo já terminou, sai
        if (game_over) return false;

        // Verifica se a posição está livre
        if (board[row][col] != ' ') {
            return false; // Posição já ocupada
        }

        // Faz a jogada
        board[row][col] = player;

        // Verifica vitória
        if (check_win(player)) {
            game_over = true;
            winner = player;
        } else if (check_draw()) {
            game_over = true;
            winner = 'D'; // Empate
        } else {
            // Alterna jogador
            current_player = (player == 'X') ? 'O' : 'X';
        }

        // Mostra tabuleiro
        display_board();

        // Notifica a outra thread
        turn_cv.notify_all();

    return true;
        
    }

    bool check_win(char player) {
        // Verifica linhas e colunas
        for (int i = 0; i < 3; ++i) {
            if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) || // linha
                (board[0][i] == player && board[1][i] == player && board[2][i] == player)) { // coluna
                return true;
            }
        }
    
        // Verifica diagonais
        if ((board[0][0] == player && board[1][1] == player && board[2][2] == player) ||  // diagonal principal
            (board[0][2] == player && board[1][1] == player && board[2][0] == player)) {  // diagonal secundária
            return true;
        }
    
        return false;
    }
        
    bool check_draw() {
        for (const auto& row : board) {
            for (char cell : row) {
                if (cell == ' ') {
                    return false; // Ainda há espaço livre
                }
            }
        }
        return true; // Nenhum espaço livre e ninguém venceu
    }

    bool is_game_over() {
        std::lock_guard<std::mutex> lock(board_mutex);
        return game_over;// Retornar se o jogo terminou
    }

    char get_winner() {
        std::lock_guard<std::mutex> lock(board_mutex);
        return winner;// Retornar o vencedor do jogo ('X', 'O', ou 'D' para empate)
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador

public:
    Player(TicTacToe& g, char s, std::string strat) 
        : game(g), symbol(s), strategy(strat) {}

    void play() {
        while (!game.is_game_over()) {
            if (strategy == "sequencial") {
                play_sequential();
            } else if (strategy == "aleatorio") {
                play_random();
            }
    
            // Pequena pausa para simular jogada e evitar corrida frenética
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }// Executar jogadas de acordo com a estratégia escolhida
    }

private:
    void play_sequential() {
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                if (game.make_move(symbol, row, col)) {
                    return; // Jogada feita
                }
            }
        }// Implementar a estratégia sequencial de jogadas
    }

    void play_random() {
        while (!game.is_game_over()) {
            int row = rand() % 3;
            int col = rand() % 3;
    
            if (game.make_move(symbol, row, col)) {
                return; // Jogada feita
            }
        }// Implementar a estratégia aleatória de jogadas
    }
};

// Função principal
int main() {
        
    srand(time(nullptr)); // Inicializa gerador aleatório

    TicTacToe jogo;

    // Cria dois jogadores: um sequencial (X) e um aleatório (O)
    Player jogador1(jogo, 'X', "sequencial");
    Player jogador2(jogo, 'O', "aleatorio");

    // Cria threads para os jogadores
    std::thread thread1(&Player::play, &jogador1);
    std::thread thread2(&Player::play, &jogador2);

    // Aguarda as threads terminarem
    thread1.join();
    thread2.join();

    // Mostra resultado final
    char resultado = jogo.get_winner();
    if (resultado == 'D') {
        std::cout << "Empate!\n";
    } else {
        std::cout << "Vencedor: " << resultado << " 🎉\n";
    }

    return 0;
    
}
