#include <input.h>
#include <iostream>
#include <scratchlangfunctions.h>
#include <string>
#ifdef _WIN32
#include <conio.h>
#include <Windows.h>
#else
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <thread>
#endif

using namespace std;

void csleep(const double t) {
    if (t > 0.0)
        this_thread::sleep_for(chrono::milliseconds((int)(1E3 * t + 0.5)));
}

char cgetch(std::string const &message) {
    if (message != "")
        std::cout << message << std::endl;
#ifdef _WIN32
    return (char)_getch();
#else
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
#endif
}

string keypress() {
    int key;
    string pressed = "";
#ifdef _WIN32
    key = (int)cgetch();
    if (key == 0 || key == -32) {
        key = (int)cgetch();
        if (key >= 59 && key <= 68)
            pressed = "F" + to_string(key - 58);
        else
            switch (key) {
            case -122:
                pressed = "F12";
                break;
            case 71:
                pressed = "Home";
                break;
            case 72:
                pressed = "UpArrow";
                break;
            case 73:
                pressed = "PageUp";
                break;
                // no 74 for some reason
            case 75:
                pressed = "LeftArrow";
                break;
                // no 76 as well
            case 77:
                pressed = "RightArrow";
                break;
                // same for 78
            case 79:
                pressed = "End";
                break;
            case 80:
                pressed = "DownArrow";
                break;
            case 81:
                pressed = "PageDown";
                break;
            case 82:
                pressed = "Insert";
                break;
            case 83:
                pressed = "Delete";
                break;
            }
    } else {
        switch (key) {
        case 8:
            pressed = "Backspace";
            break;
        case 13:
            pressed = "Enter";
            break;
        case 27:
            pressed = "Escape";
            break;
        default:
            pressed = (char)key;
            break;
        }
    }
#else
    struct termios term;
    tcgetattr(0, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &term);
    int nbbytes;
    ioctl(0, FIONREAD, &nbbytes);
    while (!nbbytes) {
        csleep(0.01);
        fflush(stdout);
        ioctl(0, FIONREAD, &nbbytes);
    }
    key = getchar();
    if (key == 27 || key == 194 || key == 195) {
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
        key = getchar();
        if (key != EOF) {
            if (key == 91) {
                key = getchar();
                if (key == 49) {
                    key = 62 + getchar();
                    if (key == 115)
                        key++;
                    getchar();
                } else if (key == 50) {
                    key = getchar();
                    if (key == 126)
                        key = 45;
                    else {
                        key += 71;
                        if (key < 121)
                            key++;
                        getchar();
                    }
                } else if (key == 51 || key == 53 || key == 54)
                    getchar();
            } else if (key == 79)
                key = 32 + getchar();
            key = -key;
        } else
            key = -27;
        fcntl(STDIN_FILENO, F_SETFL, flags);
    }
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(0, TCSANOW, &term);
    if (key <= -112 && key >= -123)
        pressed = "F" + to_string(-key - 111);
    else
        switch (key) {
        case -27:
            pressed = "Escape";
            break;
        case -45:
            pressed = "Insert";
            break;
        case -51:
            pressed = "Delete";
            break;
        case -53:
            pressed = "PageUp";
            break;
        case -54:
            pressed = "PageDown";
            break;
        case -65:
            pressed = "UpArrow";
            break;
        case -66:
            pressed = "DownArrow";
            break;
        case -67:
            pressed = "RightArrow";
            break;
        case -68:
            pressed = "LeftArrow";
            break;
        case -70:
            pressed = "End";
            break;
        case -72:
            pressed = "Home";
            break;
        case 9:
            pressed = "Tab";
            break;
        case 10:
            pressed = "Enter";
            break;
        case 127:
            pressed = "Backspace";
            break;
        default:
            pressed = (char)key;
        }
#endif
    return pressed;
}