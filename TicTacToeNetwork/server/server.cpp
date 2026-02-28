#include "../common/tictactoe.h"
#include <iostream>
#include <csignal>
#include <vector>

SOCKET serverSocketGlobal = INVALID_SOCKET;
std::vector<SOCKET> clients;
bool serverRunning = true;

void signalHandler(int signal) {
    std::cout << "\n\nВывод сообщения о завершении " << signal << ")...\n";
    serverRunning = false;
    
    for (SOCKET client : clients) {
        if (client != INVALID_SOCKET) {
            send(client, "SHUTDOWN", 8, 0);
            closesocket(client);
        }
    }
    
    if (serverSocketGlobal != INVALID_SOCKET) {
        closesocket(serverSocketGlobal);
    }
    
    WSACleanup();
    exit(signal);
}

class GameServer {
private:
    SOCKET serverSocket;
    SOCKET client1, client2;
    char board[9];
    int currentPlayer;
    bool gameRunning;

public:
    GameServer() : serverSocket(INVALID_SOCKET), client1(INVALID_SOCKET), 
                   client2(INVALID_SOCKET), currentPlayer(0), gameRunning(true) {
        for (int i = 0; i < 9; i++) board[i] = '1' + i;
    }

    bool initialize() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cout << "Ошибка\n";
            return false;
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cout << "Ишибка создания сокета\n";
            WSACleanup();
            return false;
        }

        serverSocketGlobal = serverSocket;

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(PORT);

        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cout << "Ошибка порта " << PORT << " \n";
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }

        if (listen(serverSocket, 2) == SOCKET_ERROR) {
            std::cout << "Ошибка\n";
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }

        std::cout << "╔══════════════════════════════╗\n";
        std::cout << "║   TIC-TAC-TOE SERVER         ║\n";
        std::cout << "║   Порт " << PORT << "         ║\n";
        std::cout << "║   Для выхода Ctrl+c   ║\n";
        std::cout << "╚══════════════════════════════╝\n\n";
        return true;
    }

    void run() {
        signal(SIGINT, signalHandler);
        
        std::cout << "Ожидаем игрока 1...\n";
        client1 = accept(serverSocket, NULL, NULL);
        clients.push_back(client1);
        std::cout << "Игрок 1 присоединился (X)\n";
        send(client1, "X", 1, 0);

        std::cout << "Ждем игрока 2...\n";
        client2 = accept(serverSocket, NULL, NULL);
        clients.push_back(client2);
        std::cout << "Игрок 2 присоединился (O)\n";
        send(client2, "O", 1, 0);

        std::cout << "Игра началась X ходит первым.\n\n";
        //начальное состояние
        sendBoardToAll();

        while (serverRunning && gameRunning) {
            char buffer[BUFFER_SIZE] = {0};
            int bytesReceived;

            // Получаем ход от текущего игрока
            if (currentPlayer == 0) {
                bytesReceived = recv(client1, buffer, BUFFER_SIZE, 0);
            } else {
                bytesReceived = recv(client2, buffer, BUFFER_SIZE, 0);
            }

            if (bytesReceived <= 0) {
                handleDisconnect();
                break;
            }

            if (strcmp(buffer, "выход") == 0) {
                std::cout << "Игрок " << (currentPlayer + 1) << " вышел из игры\n";
                notifyOpponentExit();
                break;
            }

            int move = atoi(buffer);
            
            // Обрабатываем ход
            if (processMove(move)) {
                // Ход успешен, отправляем обновленную доску всем
                sendBoardToAll();
                
                // Проверяем, не закончилась ли игра
                if (checkGameOver()) {
                    // Отправляем результаты
                    sendGameResults();
                    
                    // Ждем ответы на рестарт
                    char restartBuffer1[BUFFER_SIZE] = {0};
                    char restartBuffer2[BUFFER_SIZE] = {0};
                    
                    recv(client1, restartBuffer1, BUFFER_SIZE, 0);
                    recv(client2, restartBuffer2, BUFFER_SIZE, 0);
                    
                    if (restartBuffer1[0] == 'y' && restartBuffer2[0] == 'y') {
                        std::cout << "Игроки продолжают игру!\n";
                        resetGame();
                        sendBoardToAll();
                    } else {
                        std::cout << "Игроки завершают игру, выход из сервера.\n";
                        break;
                    }
                }
            }
            
        }
    }

    void sendBoardToAll() {
        send(client1, board, 9, 0);
        send(client2, board, 9, 0);
        
        //информацию о том, чей ход
        char turnMsg1 = (currentPlayer == 0) ? '1' : '0';
        char turnMsg2 = (currentPlayer == 1) ? '1' : '0';
        
        send(client1, &turnMsg1, 1, 0);
        send(client2, &turnMsg2, 1, 0);
    }

    void handleDisconnect() {
        std::cout << "Игрок отключился \n";
        notifyOpponentExit();
    }

    void notifyOpponentExit() {
        const char* exitMsg = "Выход";
        if (client1 != INVALID_SOCKET) send(client1, exitMsg, 4, 0);
        if (client2 != INVALID_SOCKET) send(client2, exitMsg, 4, 0);
    }

    bool processMove(int position) {
        if (position >= 0 && position < 9 && board[position] != 'X' && board[position] != 'O') {
            // Устанавливаем символ
            board[position] = (currentPlayer == 0) ? 'X' : 'O';
            
            std::cout << "Игрок " << (currentPlayer + 1) << " (" 
                      << (currentPlayer == 0 ? 'X' : 'O') 
                      << ") сделал ход " << (position + 1) << "\n";

            // Меняем игрока
            currentPlayer = 1 - currentPlayer;
            
            std::cout << "Смена игрока, ( " << (currentPlayer + 1) << " делает ход)\n\n";
            
            return true; 
        } else {
            std::cout << "Некорректный ход " << (currentPlayer + 1) << "\n";
            // Отправляем сообщение об ошибке текущему игроку
            const char* errorMsg = "Ошибка";
            if (currentPlayer == 0) {
                send(client1, errorMsg, 3, 0);
            } else {
                send(client2, errorMsg, 3, 0);
            }
            return false; 
        }
    }

    void sendGameResults() {
        const char* result1;
        const char* result2;
        
        if (TicTacToe::checkWin(board, 'X')) {
            result1 = "W"; // X победил
            result2 = "L"; // O проиграл
            std::cout << "Игрок Х победил! \n";
        } else if (TicTacToe::checkWin(board, 'O')) {
            result1 = "L"; // X проиграл
            result2 = "W"; // O победил
            std::cout << "Игрок О победил! \n";
        } else {
            result1 = "D"; 
            result2 = "D"; 
            std::cout << "Ничья !\n";
        }
        
        send(client1, result1, 1, 0);
        send(client2, result2, 1, 0);
    }

    bool checkGameOver() {
        return TicTacToe::checkWin(board, 'X') || 
               TicTacToe::checkWin(board, 'O') || 
               TicTacToe::isBoardFull(board);
    }

    void resetGame() {
        for (int i = 0; i < 9; i++) board[i] = '1' + i;
        currentPlayer = 0; // X начинает
    }

    ~GameServer() {
        std::cout << "\nВыключение сервера...\n";
        if (client1 != INVALID_SOCKET) closesocket(client1);
        if (client2 != INVALID_SOCKET) closesocket(client2);
        if (serverSocket != INVALID_SOCKET) closesocket(serverSocket);
        WSACleanup();
    }
};

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    GameServer server;
    if (server.initialize()) {
        server.run();
    }
    
    system("pause");
    return 0;
}