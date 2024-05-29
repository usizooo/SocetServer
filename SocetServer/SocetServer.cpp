#define WIN32_LEAN_AND_MEAN  

#include <Windows.h>         
#include <iostream>          
#include <WinSock2.h>        // Включение заголовков для работы с Windows Sockets
#include <WS2tcpip.h>        // Включение заголовков для работы с TCP/IP

using namespace std;

int main() {
    WSADATA wsaData;         // Структура для хранения информации о Windows Sockets
    ADDRINFO hints;          // Структура для хранения информации о настройках сокета
    ADDRINFO* addrResult;    // Указатель на структуру, которая будет содержать результат вызова getaddrinfo
    SOCKET ListenSocket = INVALID_SOCKET;   // Сокет для прослушивания входящих соединений, инициализируется как невалидный
    SOCKET ConnectSocket = INVALID_SOCKET;  // Сокет для соединения с клиентом, инициализируется как невалидный
    char recvBuffer[512];    // Буфер для приема данных, размером 512 байт

    const char* sendBuffer = "Hello from server";  // Сообщение, которое сервер отправляет клиенту

    // Инициализация Winsock, использование версии 2.2
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        // Если инициализация не удалась, вывод сообщения об ошибке и завершение программы
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    // Очистка структуры hints нулями
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;         // Указываем, что будем использовать IPv4
    hints.ai_socktype = SOCK_STREAM;   // Указываем, что будем использовать потоковый сокет (TCP)
    hints.ai_protocol = IPPROTO_TCP;   // Указываем, что будем использовать протокол TCP
    hints.ai_flags = AI_PASSIVE;       // Указываем, что сокет будет использоваться для прослушивания входящих соединений

    // Получение адресной информации для прослушивания на порту 666
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        // Если получение адресной информации не удалось, вывод сообщения об ошибке и завершение программы
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для addrResult
        WSACleanup();               // Завершение работы с Winsock
        return 1;
    }

    // Создание сокета для прослушивания входящих соединений
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        // Если создание сокета не удалось, вывод сообщения об ошибке и завершение программы
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для addrResult
        WSACleanup();               // Завершение работы с Winsock
        return 1;
    }

    // Привязка сокета к адресу
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        // Если привязка не удалась, вывод сообщения об ошибке и завершение программы
        cout << "Bind failed, error: " << result << endl;
        closesocket(ListenSocket);  // Закрытие сокета
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для addrResult
        WSACleanup();               // Завершение работы с Winsock
        return 1;
    }
    // Настройка сокета на прослушивание входящих соединений с максимальной очередью SOMAXCONN
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        // Если настройка на прослушивание не удалась, вывод сообщения об ошибке и завершение программы
        cout << "Listen failed, error: " << result << endl;
        closesocket(ListenSocket);  // Закрытие сокета
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для addrResult
        WSACleanup();               // Завершение работы с Winsock
        return 1;
    }
        // Ожидание подключения клиента
        ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        // Если принятие соединения не удалось, вывод сообщения об ошибке и завершение программы
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);  // Закрытие сокета прослушивания
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для addrResult
        WSACleanup();               // Завершение работы с Winsock
        return 1;
    }

    // Закрытие сокета прослушивания, так как соединение установлено
    closesocket(ListenSocket);

    // Цикл для приема данных от клиента
    do {
        ZeroMemory(recvBuffer, 512);  // Очистка буфера приема
        result = recv(ConnectSocket, recvBuffer, 512, 0);  // Получение данных от клиента
        if (result > 0) {
            // Если данные успешно получены, выводим количество полученных байт и сами данные
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            // Отправка данных обратно клиенту
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                // Если отправка не удалась, вывод сообщения об ошибке и завершение программы
                cout << "Send failed, error: " << WSAGetLastError() << endl;
                closesocket(ConnectSocket);  // Закрытие сокета
                freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для addrResult
                WSACleanup();               // Завершение работы с Winsock
                return 1;
            }
        }
        else if (result == 0) {
            // Если клиент закрыл соединение, выводим соответствующее сообщение
            cout << "Connection closing" << endl;
        }
        else {
            // Если прием данных не удался, вывод сообщения об ошибке и завершение программы
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);  // Закрытие сокета
            freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для addrResult
            WSACleanup();               // Завершение работы с Winsock
            return 1;
        }
    } while (result > 0);

    // Завершение соединения
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        // Если завершение соединения не удалось, вывод сообщения об ошибке и завершение программы
        cout << "Shutdown failed, error: " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);  // Закрытие сокета
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для addrResult
        WSACleanup();               // Завершение работы с Winsock
        return 1;
    }

    // Закрытие сокета и очистка ресурсов
    closesocket(ConnectSocket);  // Закрытие сокета соединения
    freeaddrinfo(addrResult);    // Освобождение памяти, выделенной для addrResult
    WSACleanup();                // Завершение работы с Winsock
    return 0;                    // Завершение программы
}