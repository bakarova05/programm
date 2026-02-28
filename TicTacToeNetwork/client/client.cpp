#include "../common/tictactoe.h"
#include <iostream>
#include <csignal>

SOCKET globalSocket = INVALID_SOCKET;
bool gameActive = true;

void signalHandler(int signal) {
    std::cout << "выход";
    gameActive = false;
    if (globalSocket != INVALID_SOCKET) {
        closesocket(globalSocket);
        globalSocket = INVALID_SOCKET;
    }
    WSACleanup();
    exit(signal);
}

class GameClient {
private:
    SOCKET clientSocket;
    char board[9];
    char mySymbol;
    bool myTurn;

public:
    GameClient() : clientSocket(INVALID_SOCKET), mySymbol(' '), myTurn(false) {
        for (int i = 0; i < 9; i++) board[i] = '1' + i;
    }

    bool connectToServer() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cout << "Ошибка\n";
            return false;
        }

        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            std::cout << "Успешно\n";
            WSACleanup();
            return false;
        }

        globalSocket = clientSocket;

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

        std::cout << "Ждем подключения\n";
        
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cout << "Ошибка\n";
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }

        recv(clientSocket, &mySymbol, 1, 0);
        std::cout << "Успешно, играете символом " << mySymbol << "]\n";
        std::cout << "────────────────────────\n";
        return true;
    }

    void run() {
        signal(SIGINT, signalHandler);
        bool gameRunning = true;

        while (gameRunning && gameActive) {
            // Доска
            int bytesReceived = recv(clientSocket, board, 9, 0);
            if (bytesReceived <= 0) {
                std::cout << "Сервер отключен.\n";
                break;
            }

            // Информация чей ход
            char turnBuffer;
            bytesReceived = recv(clientSocket, &turnBuffer, 1, 0);
            if (bytesReceived <= 0) {
                std::cout << "Сервер отключен.\n";
                break;
            }
            
            myTurn = (turnBuffer == '1');

            // Очищаем экран и показываем доску
            TicTacToe::printBoard(board);

            // Проверяем, не закончилась ли игра 

            if (myTurn) {
                //запрашиваем ввод
                std::cout << "\n Ваш ход [" << mySymbol << "] \n";
                std::cout << "Сделай ход (1-9) или 0 для завершения игры: ";
                
                int move;
                std::cin >> move;
                
                if (move == 0) {
                    std::cout << "Завершаю...\n";
                    send(clientSocket, "Выход", 4, 0);
                    break;
                }
                
                move--; // Преобразуем в индекс 0-8
                
                if (move < 0 || move > 8) {
                    std::cout << "Ошибка! Выбери от 1 до 9\n";
                    std::cout << "Нажми Enter для продолжения...";
                    std::cin.ignore();
                    std::cin.get();
                    continue;
                }
                
                // Отправляем ход на сервер
                std::string moveStr = std::to_string(move);
                send(clientSocket, moveStr.c_str(), moveStr.length(), 0);
                std::cout << "Ждем оппонента...\n";
                
                
            } else {
                
                std::cout << "\n Ход противника [" 
                          << (mySymbol == 'X' ? 'O' : 'X') << "] \n";
                std::cout << "Ждём... \n";
                std::cout << " Ctrl+C для выхода \n";
                
               
            }

          
        }
    }

    ~GameClient() {
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
        }
        WSACleanup();
    }
};

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    GameClient client;
    if (client.connectToServer()) {
        client.run();
    }
    
    system("pause");
    return 0;
}