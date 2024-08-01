#include <iostream>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <ncurses.h>
#include <ctime>

//TODO get col value, set max input length to that value - name/date

WINDOW* output_win;
WINDOW* input_win;

// message structure
struct Message {
    std::string sender;
    std::string message;
    std::string date;
};

// initialize ncurses
void init_ncurses() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
}

// end ncurses
void end_ncurses() {
    endwin();
}

// set message details
Message set_message(Message* message, const std::string& mess, const std::string& name) {
    std::time_t raw_time = std::time(nullptr);
    std::string date = std::ctime(&raw_time);

    if (!date.empty() && date.back() == '\n') {
        date.pop_back();
    }

    std::string day_date = date.substr(4, 6);
    if (day_date[4] == ' ') {
        day_date[4] = '0';
    }
    std::string day_time = "[" + day_date + " " + date.substr(11, 5) + "]";

    message->date = day_time;
    message->message = mess;
    message->sender = name;

    return *message;
}

// receive messages from server
void receive_messages(int client_socket) {
    char buffer[1024];
    std::string accumulated_data;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "Error receiving data or connection closed by server" << std::endl;
            break;
        }

        accumulated_data.append(buffer, bytes_received);

        size_t pos;
        while ((pos = accumulated_data.find('\n')) != std::string::npos) {
            std::string message = accumulated_data.substr(0, pos + 1);
            accumulated_data.erase(0, pos + 1);

            if (!message.empty() && message.back() == '\n') {
                message.pop_back();
            }

            wmove(output_win, getcury(output_win), 1);
            wprintw(output_win, "%s\n", message.c_str());
            wrefresh(output_win);
			
            wmove(input_win, 1, 1);
            wclrtoeol(input_win);
            wrefresh(input_win);
        }
    }
}

// initialize window
void init_window() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    output_win = newwin(rows - 3, cols, 0, 0);
    input_win = newwin(3, cols, rows - 3, 0);

    scrollok(output_win, TRUE);

    box(output_win, 0, 0);
    box(input_win, 0, 0);
    wrefresh(output_win);
    wrefresh(input_win);
}

// print welcome message
void print_welcome() {
    wmove(output_win, getcury(output_win), 1);
    wprintw(output_win, "Welcome to the chat!\n");
    wrefresh(output_win);
    box(input_win, 0, 0);
}

// create and connect socket
int set_connect_socket() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Error creating the socket ..." << std::endl;
        return -1;
    }

    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(4000);
    clientAddr.sin_addr.s_addr = inet_addr("192.168.65.134");

    if (connect(client_socket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == -1) {
        std::cerr << "Connection failed ruh roh" << std::endl;
        return -1;
    }

    return client_socket;
}

// send message to server
void send_message(int client_socket, const std::string& message) {
    send(client_socket, message.c_str(), message.size(), 0);
}

// main function
int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <name>" << std::endl;
        return 1;
    }

    std::string name = argv[1];

    init_ncurses();
    init_window();
    print_welcome();

    int client_socket = set_connect_socket();
    if (client_socket == -1) {
        end_ncurses();
        return 1;
    }

    // receive thread (for simultaneous send/receive non-blocking io)
    std::thread rec_thread([client_socket]() { receive_messages(client_socket); });
    rec_thread.detach();

    char mess[1024];
    while (true) {
        wmove(input_win, 1, 1);
        wclrtoeol(input_win);
        box(input_win, 0, 0);
        wrefresh(input_win);
		
        echo();
        wgetnstr(input_win, mess, sizeof(mess) - 1);
        noecho();

        Message message;
        set_message(&message, mess, name);
        std::string complete_message = message.date + " " + message.sender + ": " + message.message + "\n";

        wmove(output_win, getcury(output_win), 1);
        wprintw(output_win, complete_message.c_str());
        box(output_win, 0, 0);
        wrefresh(output_win);

        send_message(client_socket, complete_message);

        if (strcmp(mess, "exit\n") == 0)
            break;
        memset(mess, 0, sizeof(mess));
    }

    close(client_socket);
    end_ncurses();

    return 0;
}
