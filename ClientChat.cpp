#include <iostream>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <functional>
#include <ncurses.h>

//TODO create message struct with buffer and username and time

WINDOW* output_win;
WINDOW* input_win;


void receive_messages(int client_socket)
{
    char buffer[1024];
	std::string accumulated_data;

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
        {
            std::cerr << "Error recoeving data or connection closed by server" << std::endl;
			break;
        }

		accumulated_data.append(buffer, bytes_received);

        size_t pos;
        while ((pos = accumulated_data.find('\n')) != std::string::npos)
        {
            std::string message = accumulated_data.substr(0, pos + 1);
            accumulated_data.erase(0, pos + 1);

            if (!message.empty() && message.back() == '\n') {
                message.pop_back();
            }

            wmove(output_win, getcury(output_win), 1);
            wprintw(output_win, "Buffer: %s\n", message.c_str());
            wrefresh(output_win);
        }				

    }
}

void init_ncurses() {
    initscr();
    cbreak();
    noecho();
    //curs_set(2);
    keypad(stdscr, TRUE);
}

void end_ncurses() {
    endwin();
}


int main(int argc, char *argv[])
{
    init_ncurses();

	char name[1024];

    //get terminal window size
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

	output_win = newwin(rows - 3, cols, 0, 0);
    input_win = newwin(3, cols, rows - 3, 0);

    //grab dimensions of current window, following lines structure window
    scrollok(output_win, TRUE);
    //wborder(inputWin, '|', '|', '-', '-', '+', '+', '+', '+'); //draws the border


    box(output_win, 0, 0); //box around input window
    box(input_win, 0,0);
    wrefresh(output_win); //refresh with new settings
    wrefresh(input_win);

    wmove(output_win, getcury(output_win), 1);
	wprintw(output_win, "Welcome to the chat!\n");
	wrefresh(output_win);

    //eventually QString name will = argv[0], pass into an alias instead of declaring username every time

    //output << "\033[1;32m" << "This is a green message" << "\033[0m" << Qt::endl;
    //output << "\033[1;31m" << "This is a red message" << "\033[0m" << Qt::endl;
    //output << "\033[1;34m" << "This is a blue message" << "\033[0m" << Qt::endl;



    //output << "Hello, " << name << "!" << Qt::endl;
    //std::cout << "Starting the Client ..." << std::endl;
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        std::cerr << "Error creating the socket ..." << std::endl;
    }
    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(4000);
    clientAddr.sin_addr.s_addr = inet_addr("192.168.65.134");

    //probably add error handling here, jk prob not :)
    if (connect(client_socket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == -1)
    {
            std::cerr << "connection failed ruh roh" << std::endl;
    }

    //std::cout << "connection successfull" << std::endl;

    //recieve thread (for simultaneous send/receive non-blocking io)
    std::thread rec_thread([client_socket]() { receive_messages(client_socket); });
    rec_thread.detach();


    //output << "\033[1;32m" << "Welcome to the Qt Console App!\n" << "\033[0m" << Qt::endl;
    //output << "Please enter your name: ";
    //output.flush();

    //QString name;
    //input >> name;


    char mess[1024];
    while (true)
    {

        wmove(input_win, 1, 1);
        wclrtoeol(input_win);
        wrefresh(input_win);
		
		echo();
        wgetnstr(input_win, mess, sizeof(mess) - 1);
		noecho();

		wmove(output_win, getcury(output_win), 1);
        wprintw(output_win, "You entered: %s\n", mess); //make this your name input
        wrefresh(output_win);

        //fgets(mess, sizeof(mess), stdin); //fix input method

        //waddstr(inputWin, "Enter message: ");
        //wgetnstr(inputWin, mess, 255);

        send(client_socket, &mess, strlen(mess), 0);
        if (strcmp(mess, "exit\n") == 0)
            break;
        memset(mess, 0, sizeof(mess));
    }
    close(client_socket);

    // Start the event loop
	return 0;
}

