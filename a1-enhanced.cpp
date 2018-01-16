
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <math.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h> // main Xlib header
//#include "simon.h"
using namespace std;

const int Border = 1;
const int BufferSize = 10;
const int FPS = 60;
bool wiggleflag = true;

/*
 * Information to draw on the window.
 */
struct XInfo {
    Display*     display;
    int         screen;
    Window     window;
    GC         gc;
    // Pixmap pixmap;
    int width;
    int height;
};

struct buttons {
    int x;
    int y;
};

int sinfunct (int x) {
    return sin(x)/2*M_PI;
}

// get microseconds
unsigned long now() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

XInfo xinfo;
struct buttons* buttons = new struct buttons[5];

/*
 * Function to put out a message on error exits.
 */
void error( string str ) {
    cerr << str << endl;
    exit(0);
}

// convert to string
string toString(int i) {
    stringstream ss;
    ss << i;
    return ss.str();
}

/*
 * An abstract class representing displayable things.
 */
class Displayable {
public:
    virtual void paint(XInfo &xinfo) = 0;
};

class Text : public Displayable {
public:
    virtual void paint(XInfo &xinfo){
        XDrawImageString(xinfo.display, xinfo.window, xinfo.gc,
                         this->x, this->y, this->s.c_str(),
                         this->s.length() );
    }
    // constructor
    Text(int x, int y, string s)
    : x(x), y(y), s(s)
    {}
private:
    int x;
    int y;
    string s;
};

void repaint( list<Displayable*> dList, XInfo& xinfo);
/*
 * A Button displayable
 */
class Button : public Displayable {
public:
    virtual void paint(XInfo& xinfo) {
        // create a simple graphics context
        GC gc = XCreateGC(xinfo.display, xinfo.window, 0, 0);
        int screen = DefaultScreen( xinfo.display );
        XSetLineAttributes(xinfo.display, gc,
                           ol,       // 3 is line width
                           LineSolid, CapButt, JoinRound);     // other line options*/s

            XDrawArc(xinfo.display, xinfo.window, gc,
                     x - (d / 2), y - (d / 2), d, d, 0, 360 * 64);
        XSetForeground(xinfo.display, gc, WhitePixel(xinfo.display, screen));
    }
    // constructor
    Button(int x, int y, int d, int ol): x(x), y(y), d(d), ol(ol) {}
private:
    int x;
    int y;
    int d; // diameter
    int ol;
};

class WhiteCircle : public Displayable {
public:
    virtual void paint(XInfo& xinfo) {
        // create a simple graphics context
        GC gc = XCreateGC(xinfo.display, xinfo.window, 0, 0);
        int screen = DefaultScreen( xinfo.display );
        XSetForeground(xinfo.display, gc, WhitePixel(xinfo.display, screen));
        XSetBackground(xinfo.display, gc, WhitePixel(xinfo.display, screen));
        XSetLineAttributes(xinfo.display, gc,
                           1,       // 3 is line width
                           LineSolid, CapButt, JoinRound);    // other line options*/s

        XDrawArc(xinfo.display, xinfo.window, gc,
                 x - (d / 2), y - (d / 2), d, d, 0, 360 * 64);
    }
    // constructor
    WhiteCircle(int x, int y, int d): x(x), y(y), d(d) {}
private:
    int x;
    int y;
    int d; // diameter
};

int needcircle = 0;
bool ifspacepressed = 0;

void handleResize(XInfo &xinfo, XEvent &event) {
    XConfigureEvent xce = event.xconfigure;
    if (xce.width != xinfo.width || xce.height != xinfo.height) {
        xinfo.width = xce.width;
        xinfo.height = xce.height;
    }
}

void handleMotion(XInfo &xinfo, XEvent &event,list<Displayable*> dList, int inside, int N) {
    while (!inside) {
        //dList.clear();
        if (ifspacepressed) {
            dList.push_back(new Text(50, 50, "0"));
            dList.push_back(new Text(50, 100, "Watch what I do . . ."));
        }
        else {
            dList.push_back(new Text(50, 50, "0"));
            dList.push_back(new Text(50, 100, "Press SPACE to play"));
        }
        for (int i=0; i<N; i++) {
            int x = ((i+1) * ((xinfo.width - (100*N)) / (N+1))) + (i*100) + 50;
            int y = xinfo.height / 2;
            string s = toString(i+1);
            if (sqrt(pow(event.xmotion.x - x, 2) + pow (event.xmotion.y - y,2)) < 50) {
                dList.push_back(new Button(x, y, 100,4));
            } else {
                dList.push_back(new Button(x, y, 100,1));
            }
            
            dList.push_back(new Text(x-4, y+10, s));
        }
        repaint(dList, xinfo);
        return;
    }
}

void handlepress (XInfo &xinfo,list<Displayable*> dList, int x, int y, int d, int N);

void handleButtonPress(XInfo &xinfo, XEvent &event,list<Displayable*> dList, int inside, int N) {
    int xx, yy;
    while (!inside) {
        if (ifspacepressed) {
            dList.push_back(new Text(50, 50, "0"));
            dList.push_back(new Text(50, 100, "Watch what I do . . ."));
        } else {
            dList.push_back(new Text(50, 50, "0"));
            dList.push_back(new Text(50, 100, "Press SPACE to play"));
        }
        for (int i=0; i<N; i++) {
            int x = ((i+1) * ((xinfo.width - (100*N)) / (N+1))) + (i*100) + 50;
            int y = xinfo.height / 2;
            string s = toString(i+1);
            if (sqrt(pow(event.xmotion.x - x, 2) + pow (event.xmotion.y - y,2)) < 50) {
                dList.push_back(new Button(x, y, 1,100));
                needcircle = 1;
                xx = x;
                yy = y;
            } else {
                dList.push_back(new Button(x, y, 100,1));
            }
        }
        /* if (needcircle) {
         // handlepress(xinfo, dList, xx, yy, 100, N);
         // repaint(dList, xinfo);
         needcircle = 0;
         return;
         }*/
        repaint(dList, xinfo);
        return;
    }
}

//int d = 100;
void handlepress (XInfo &xinfo, list<Displayable*> dList, int x, int y, int d, int N) {
    
    GC gc = XCreateGC(xinfo.display, xinfo.window, 0, 0);
    // time of last window paint
    unsigned long lastRepaint = 0;
    XWindowAttributes w;
    XGetWindowAttributes(xinfo.display, xinfo.window, &w);
    while (needcircle) {
        unsigned long end = now();
        if (end - lastRepaint > 1000000 / FPS) {
            // clear background
            XClearWindow(xinfo.display, xinfo.window);

            XDrawString(xinfo.display, xinfo.window, xinfo.gc,50, 50, "0", 1);
            XDrawString(xinfo.display, xinfo.window, xinfo.gc,50, 100, "Press SPACE to play",19 );
            for (int i=0; i<N; i++) {
                int x2 = ((i+1) * ((xinfo.width - (100*N)) / (N+1))) + (i*100) + 50;
                int y2 = xinfo.height / 2;
                string s = toString(i+1);
                if (x2 == x && y2 == y) {
                    dList.push_back(new Button(x2, y2, 1,100));

                } else {
                    dList.push_back(new Button(x2, y2, 100,1));
                }
            }
            // repaint(dList, xinfo);
            //XDrawArc(xinfo.display, xinfo.window, gc, x - (d / 2), y - (d / 2), d, d, 0, 360 * 64);
            dList.push_back(new WhiteCircle(x, y, d));
            d = d - 5;
            repaint(dList, xinfo);
            XFlush( xinfo.display );
            lastRepaint = now(); // remember when the paint happened
            
        }
        // IMPORTANT: sleep for a bit to let other processes work
        if (XPending(xinfo.display) == 0) {
            usleep(1000000 / FPS - (end - lastRepaint));
        }
    }
    
}

void handleKeyPress(XInfo &xinfo, XEvent &event,list<Displayable*> dList, int N) {
    KeySym key;
    char text[BufferSize];
    int i = XLookupString(
                          (XKeyEvent*)&event, text, BufferSize, &key, NULL );
    cout << "KeySym " << key
    << " text='" << text << "'"
    << " at " << event.xkey.time
    << endl;
    if (i == 1 && text[0] == 'q' ) {
        cout << "Terminated normally." << endl;
        XCloseDisplay(xinfo.display);
        return;
    }
    if (i == 1 && text[0] == ' ') {
        ifspacepressed = 1;
        dList.push_back(new Text(50, 50, "0"));
        dList.push_back(new Text(50, 100, "Watch what I do . . ."));
        for (int i=0; i<N; i++) {
            int x = ((i+1) * ((xinfo.width - (100*N)) / (N+1))) + (i*100) + 50;
            int y = xinfo.height / 2;
            string s = toString(i+1);
            dList.push_back(new Button(x, y, 100,1));
            dList.push_back(new Text(x-4, y+10, s));
        }
        repaint(dList, xinfo);
        return;
    }
}

// Function to repaint a display list
void repaint( list<Displayable*> dList, XInfo& xinfo) {
    list<Displayable*>::const_iterator begin = dList.begin();
    list<Displayable*>::const_iterator end = dList.end();
    XClearWindow(xinfo.display, xinfo.window);
    //XSetBackground(xinfo.display, xinfo.gc, WhitePixel(xinfo.display, xinfo.screen));
    // XFillRectangle(xinfo.display, xinfo.window, xinfo.gc, 0, 0, xinfo.width, xinfo.height);
    while( begin != end ) {
        Displayable* d = *begin;
        d->paint(xinfo);
        begin++;
    }
    XFlush(xinfo.display);
}


// The loop responding to events from the user.
void eventloop(XInfo& xinfo, int N) {
    XEvent event;
    KeySym key;
    char text[BufferSize];
    list<Displayable*> dList;
    Button* button = NULL;
    int inside = 0;
    // create the Simon game object
    //Simon simon = Simon(N, true);
    
    while ( true ) {
        XNextEvent( xinfo.display, &event );
        dList.push_back(new Text(50, 50, "0"));
        if (ifspacepressed) {
            dList.push_back(new Text(50, 50, "0"));
            dList.push_back(new Text(50, 100, "Watch what I do . . ."));
        } else {
            dList.push_back(new Text(50, 50, "0"));
            dList.push_back(new Text(50, 100, "Press SPACE to play"));
        }
        
        for (int i=0; i<N; i++) {
            int x = ((i+1) * ((xinfo.width - (100*N)) / (N+1))) + (i*100) + 50;
            int y = xinfo.height / 2;
            string s = toString(i+1);
            buttons[i].x = x;
            buttons[i].y = y;
            dList.push_back(new Button(x, y, 100,1));
            dList.push_back(new Text(x-4, y+10, s));
        }
        repaint(dList, xinfo);
        
        switch ( event.type ) {
            case ConfigureNotify:
                dList.clear();
                handleResize(xinfo, event);
                break;
            case MotionNotify:
                dList.clear();
                handleMotion(xinfo, event, dList, inside, N);
                break;
            case ButtonPress:
                //dList.clear();
                handleButtonPress(xinfo, event, dList, inside, N);
                break;
          /*  case Expose:
                cout << "Expose count " <<
                event.xexpose.count << endl;
                if ( event.xexpose.count == 0 ) {
                    repaint( dList, xinfo);
                }
                break;*/
            case KeyPress:
                dList.clear();
                handleKeyPress(xinfo, event, dList, N);
                break;
        }
        //usleep(/FPS);
        // handleAnimation(xinfo, inside);
        // repaint(dList, xinfo);
    }
}

/*
 * Create a window
 */
void initX(int argc, char* argv[], XInfo& xinfo) {
    /*
     * Display opening uses the DISPLAY    environment variable.
     * It can go wrong if DISPLAY isn't set, or you don't have permission.
     */
    xinfo.display = XOpenDisplay( "" );
    if ( !xinfo.display )    {
        error( "Can't open display." );
    }
    /*
     * Find out some things about the display you're using.
     */
    // DefaultScreen is as macro to get default screen index
    xinfo.screen = DefaultScreen( xinfo.display );
    xinfo.height = 400;
    xinfo.width = 800;
    unsigned long white, black;
    white = XWhitePixel( xinfo.display, xinfo.screen );
    black = XBlackPixel( xinfo.display, xinfo.screen );
    xinfo.window = XCreateSimpleWindow(
                                       xinfo.display,          // display where window appears
                                       DefaultRootWindow( xinfo.display ), // window's parent in window tree
                                       10, 10,                  // upper left corner location
                                       xinfo.width, xinfo.height,                // size of the window
                                       5,                       // width of window's border
                                       black,                   // window border colour
                                       white );                 // window background colour
    // extra window properties like a window title
    XSetStandardProperties(
                           xinfo.display,        // display containing the window
                           xinfo.window,        // window whose properties are set
                           "a1",                // window's title
                           "a1",                // icon's title
                           None,                // pixmap for the icon
                           argv, argc,            // applications command line args
                           None );                // size hints for the window
    
    
    // drawing demo with graphics context here ...
    xinfo.gc = XCreateGC(xinfo.display, xinfo.window, 0, 0);       // create a graphics context
    XSetForeground(xinfo.display, xinfo.gc, BlackPixel(xinfo.display, xinfo.screen));
    XSetBackground(xinfo.display, xinfo.gc, WhitePixel(xinfo.display, xinfo.screen));
    //load a larger font
    XFontStruct * font;
    font = XLoadQueryFont (xinfo.display, "12x24");
    XSetFont (xinfo.display, xinfo.gc, font->fid);
    
    XSelectInput(xinfo.display, xinfo.window,
                 ButtonPressMask | KeyPressMask |
                 PointerMotionMask | StructureNotifyMask);
    
    //XSetWindowBackgroundPixmap(xinfo.display, xinfo.window, None);
    
    /*
     * Put the window on the screen.
     */
    XMapRaised( xinfo.display, xinfo.window );
    XFlush(xinfo.display);
    
    //sleep(1);
    
    // give server 10ms to get set up before sending drawing commands
    //usleep(10 * 1000);
    
}



int main ( int argc, char* argv[] ) {
    
    int n = 4;
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    n = max(1, min(n, 9));
    cout << "press q to exit" << "\n";
    XInfo xinfo;
    initX(argc, argv, xinfo);
    eventloop(xinfo,n);
    // wait for user input to quit (a concole event for now)
    
    //cin.get();
   // XCloseDisplay(xinfo.display);
    
}
