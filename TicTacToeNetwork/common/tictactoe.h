#pragma once
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 5555;
const int BUFFER_SIZE = 1024;
const char SERVER_IP[] = "127.0.0.1"; 

class TicTacToe {
public:
    static void printBoard(const char board[9]) {
        system("cls"); // Очистка консоли 
        std::cout << "\n\n";
        std::cout << "     |     |     \n";
        std::cout << "  " << board[0] << "  |  " << board[1] << "  |  " << board[2] << "  \n";
        std::cout << "_____|_____|_____\n";
        std::cout << "     |     |     \n";
        std::cout << "  " << board[3] << "  |  " << board[4] << "  |  " << board[5] << "  \n";
        std::cout << "_____|_____|_____\n";
        std::cout << "     |     |     \n";
        std::cout << "  " << board[6] << "  |  " << board[7] << "  |  " << board[8] << "  \n";
        std::cout << "     |     |     \n\n";
    }

    static bool checkWin(const char board[9], char player) {
        // Проверка горизонталей
        for (int i = 0; i < 9; i += 3) {
            if (board[i] == player && board[i + 1] == player && board[i + 2] == player)
                return true;
        }
        // Проверка вертикалей
        for (int i = 0; i < 3; i++) {
            if (board[i] == player && board[i + 3] == player && board[i + 6] == player)
                return true;
        }
        // Проверка диагоналей
        if (board[0] == player && board[4] == player && board[8] == player)
            return true;
        if (board[2] == player && board[4] == player && board[6] == player)
            return true;
        return false;
    }

    static bool isBoardFull(const char board[9]) {
        for (int i = 0; i < 9; i++) {
            if (board[i] != 'X' && board[i] != 'O')
                return false;
        }
        return true;
    }
};