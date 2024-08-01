#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <mutex>
#include <algorithm>
#include <thread>
#include <vector>

//TODO - why do chat logs have line returns

std::mutex clients_mutex;
std::vector<int> client_sockets;

void handle_client(int client_socket)
{
    std::string chat_line;
    std::fstream chat_logs("chat_logs", std::ios::in | std::ios::out | std::ios::app);
    std::cout << "_____Message History Below_____" << std::endl;
    while (getline(chat_logs, chat_line))
    {
	if (!chat_line.empty())
	{
	    chat_line += "\n";
	    send(client_socket, chat_line.c_str(), chat_line.size(), 0);
	}
    }
    chat_logs.close();

    char buff[1024];
    std::fstream chat_logs_append("chat_logs", std::ios::app);
    while (true)
    {
        memset(buff, 0, sizeof(buff));
        int bytes_received = recv(client_socket, buff, sizeof(buff), 0);
        if (bytes_received <= 0)
        {
            std::cerr << "Error receiving data or connection closed by client." << std::endl;
            break;
        }
	std::string message(buff, bytes_received);
        std::cout << message << std::endl;;

        //write to chat logs
        chat_logs_append << buff << std::endl;

	{
	    std::lock_guard<std::mutex> lock(clients_mutex);
	    //iterate through all connected sockets(clients) and send the message to each except the og sender
	    for (int sock : client_sockets)
            {
                if (sock != client_socket)

		{
		    send(sock, message.c_str(), message.size(), 0);
		}
	    }
	}
        if (strcmp(buff, "exit\n") == 0)
            break;
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
	client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
    }
    close(client_socket);
}

int main()
{
    std::string chat_line;
    std::cout << "Starting the server ..." << std::endl;

    std::fstream chat_logs("chat_logs", std::ios::in | std::ios::out | std::ios::app);
    if (!chat_logs.is_open())
    {
        std::cerr << "Error opening chat log file!" << std::endl;
        return 1;
    }

    std::cout << "_____Message History Below_____" << std::endl;
    while (getline(chat_logs, chat_line))
    {
	if (chat_line != "")
	{
            std::cout << chat_line << std::endl;
	}
    }
    chat_logs.close();

    //create socket (af_inet = ipv4, sock_stream = tcp, 0 is default for something idk
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cerr << "Error creating the socket!" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    //set the server connection family to ipv4
    serverAddr.sin_family = AF_INET;
    //set the server to listen form any incoming ip and not a specific one
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(4000);


    //binds our server socket to aforementioned server address, also casts serverAddr (sockaddr_in)
    //to a type sockadr struct
    if (bind(server_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << "Error binding the socket!" << std::endl;
        close(server_socket);
        return 1;
    }

    //sets socket up to listen for connections, takes in server socket and max queue
    if (listen(server_socket, 5) == -1)
    {
        std::cerr << "Error listening on the socket!" << std::endl;
        close(server_socket);
        return 1;
    }

    //after setting up listen parameters, this will accept connections from the listen queue
    //if the queue is empty, it wont return until it recieves a connection

    char buff[1024];
    std::fstream chat_logs_append("chat_logs", std::ios::app);
    while (true)
    {

        int client_socket = accept(server_socket, NULL, NULL);
	if (client_socket == -1)
	{
	std::cerr << "Error accepting connection!" << std::endl;
	close(server_socket);
	return 1;
    	}

	{
	    std::lock_guard<std::mutex> lock(clients_mutex);
	    client_sockets.push_back(client_socket);
	}

	std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);
    return 0;
}