#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 800;

enum textFields {
    FILENAME,
    TEXTFIELD
};

enum ButtonFunctions{
    READ,
    WRITE,
    QUIT,
    NONE
};

SDL_Window* window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );

class IOfile{
    public:
    string name;

    IOfile(string name){
        this->name = name; 
    }

    void openAndRead(vector<string> &lines){
        ifstream is (name, std::ifstream::binary);
        if (is) {
            
            is.seekg (0, is.end);
            int length = is.tellg();
            is.seekg (0, is.beg);

            char * buffer = new char [length];

            std::cout << "reading " << length << " characters from " << name << endl;
            
            is.read (buffer,length);

            if (is)
            std::cout << "all characters read successfully" << endl;
            else
            std::cout << "error: only " << is.gcount() << " could be read" << endl;
            is.close();

            int currLine = 0;
            string tmp = "";

            lines.clear();

            for (int i = 0; i < length; i++){
                if (buffer[i]=='\n' or buffer[i]=='\0'){
                    lines.push_back(tmp); 
                    tmp = "";
                }
                else{
                    tmp += buffer[i];
                }
            }
            lines.push_back(tmp);

            delete buffer;
            
        }
        
    }

    void openAndWrite(vector<string> &lines){
        ofstream is (name, std::ofstream::binary);
        if (is) {

            char * buffer;  
            std::cout << "writing " << lines.size() << " lines to " << name << endl;  
            for (int i = 0; i < lines.size(); i++){
                buffer = new char [lines[i].length()+1];
                strcpy(buffer, lines[i].c_str());

                is.write(buffer, lines[i].length());
                buffer[0] = '\n';
                is.write(buffer, 1);                
            }

            std::cout << "all lines written successfully" << endl;

            delete buffer;
           
            is.close();
        }
        
    }
};

struct Click{
    int xpos;
    int ypos;
};


class textField{

    private:
    SDL_Rect cursorRect;    
    TTF_Font * font;
    SDL_Color color; 
    bool isEditable;
    int font_width;
    int font_height;
    SDL_Rect borderRect;
    IOfile* assignedFile;

    public:
    friend class HostFrame;
    int xStartPos, yStartPos;
    vector<string> lines;
    string text;
    int cursorStringPosition;  
    int currentLine;
    bool isFileChanger = false;  

    textField(int xStartPos, int yStartPos, int fontsize, SDL_Color color){
        this->xStartPos = xStartPos;
        this->yStartPos = yStartPos;
        this->color = color;
        this->isEditable = false;
        
        
        currentLine = 0;

        borderRect.x = 0;
        borderRect.y = yStartPos - 5;
        borderRect.w = SCREEN_WIDTH;
        borderRect.h = SCREEN_HEIGHT; 

        //-----
        font_height = 20;
        font_width = 16;

        assignedFile = NULL;

        font = TTF_OpenFont("arial.ttf", fontsize);

    }
    textField(int xStartPos, int yStartPos, int fontsize, SDL_Color color, IOfile* assignedFile){
        this->xStartPos = xStartPos;
        this->yStartPos = yStartPos;
        this->color = color;
        this->isEditable = true;
        
        currentLine = 0;

        borderRect.x = 0;
        borderRect.y = yStartPos - 5;
        borderRect.w = SCREEN_WIDTH;
        borderRect.h = SCREEN_HEIGHT; 

        //-----
        font_height = 20;
        font_width = 16;
    
        cursorStringPosition = 0;
        cursorRect = {xStartPos, yStartPos, 5, 30};
        this->assignedFile = assignedFile;

        font = TTF_OpenFont("arial.ttf", fontsize);
    }   

    void render(){
        SDL_Surface* surface = new SDL_Surface;
        SDL_Texture* texture = NULL; 

        if(isEditable){
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &borderRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }

        for (int i = 0; i < lines.size(); i++) {    
            char *cstr = new char[lines[i].length()+1];
            strcpy(cstr, lines[i].c_str());


            for(int i = 0; i < lines[i].length(); i++){
                if (cstr[i] == '\n'){
                    cstr[i] = '\0';
                }
            }

            if (isEditable){
                SDL_SetRenderDrawColor( renderer, 0xAA, 0xAA, 0xAA, 0xFF );
                cursorRect.x = xStartPos + cursorStringPosition*font_width;
                cursorRect.y = yStartPos + currentLine*font_height;
                SDL_RenderFillRect(renderer, &cursorRect);
            }

            if(lines[i].length()){    
                surface = TTF_RenderText_Solid(font, cstr, color);
                texture = SDL_CreateTextureFromSurface(renderer, surface);
                int texW = 0;
                int texH = 0;
                int x = xStartPos;
                int y = yStartPos;
                SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);

                if (!isEditable){
                    //center
                    x = xStartPos + texW/2;
                }

                SDL_Rect dstrect = { x, y+i*font_height, texW, texH };
                SDL_RenderCopy(renderer, texture, NULL, &dstrect);
            }

            delete cstr;
        }

        
        delete surface;
    }

    void readFromFile(){
        lines.clear();
        assignedFile->openAndRead(lines);
        currentLine = lines.size()-1;
        cursorStringPosition = lines.back().length();        
    }

    void writeToFile(){
        assignedFile->openAndWrite(lines);
    }

    virtual void enterKey(){
        lines.push_back("");
        cursorStringPosition = 0;
        currentLine++;
    }

    void handleEvent(SDL_Event &e){
        if (e.type == SDL_KEYDOWN ){
            switch(e.key.keysym.sym){
                case SDLK_RETURN:
                    if(isFileChanger){

                    }

                    else{
                        lines.push_back("");
                        cursorStringPosition = 0;
                        currentLine++;
                            
                    }
                    break;
                    
                case SDLK_DELETE:
                    if( lines[currentLine].length() > 0){
                        lines[currentLine].pop_back();
                        if( cursorStringPosition > lines[currentLine].length() &&
                             cursorStringPosition > 0){
                             cursorStringPosition--;
                        } 
                    }

                    else if (lines.size()>1){
                        lines.erase(lines.begin() + currentLine);
                        if(currentLine > lines.size()-1){
                            currentLine = lines.size()-1;
                        }
                        cursorStringPosition = lines[currentLine].size();
                    }
                    
                    
                    break;
                case SDLK_LEFT:
                    if(cursorStringPosition > 0){
                        cursorStringPosition--;
                    } 
                    break;
                case SDLK_RIGHT:
                    if(cursorStringPosition < lines[currentLine].length()){
                        cursorStringPosition++;
                    } 
                    break;

                case SDLK_UP:
                    if(currentLine > 0){
                        currentLine--;
                        cursorStringPosition = lines[currentLine].size();
                    }
                    break;
                case SDLK_DOWN:
                    if(currentLine < lines.size()-1){
                        currentLine++;
                        cursorStringPosition = lines[currentLine].size();
                    }
                    break;                          
            }
        }

        else if (e.type == SDL_TEXTINPUT){
            lines[currentLine].insert(cursorStringPosition, e.text.text);
            cursorStringPosition++;
        }
    }
    
};
// button constructor makes the button size from querytexture on label

class Button{
    public:
    SDL_Rect rect;
    string label;
    bool isClicked;
    int fontsize;
    ButtonFunctions function;
    textField* assignedField = NULL;
    textField* labelField = NULL;

    Button(){

    }

    Button(string label, ButtonFunctions function){
        isClicked = false;
        this->label = label;
        fontsize = 17;
        rect.h = 40;
        rect.w = 100;
        rect.x = 30;
        rect.y = 30;

        this->assignedField = NULL;
        this->function = function;

        this->labelField = new textField(rect.x, rect.y, fontsize, {0, 0, 0});
        labelField->lines.push_back( label );

    }

    Button(string label, ButtonFunctions function, textField* assignedField){
        isClicked = false;
        this->label = label;
        fontsize = 17;
        rect.h = 40;
        rect.w = 100;
        rect.x = 30;
        rect.y = 30;

        this->assignedField = assignedField;
        this->function = function;

        this->labelField = new textField(rect.x, rect.y, fontsize, {0, 0, 0});
        labelField->lines.push_back( label );
    }    

    void onClick(bool &quit){

        switch (function){
            case READ:
                assignedField->readFromFile();
                break;
            case WRITE:
                assignedField->writeToFile();
                break;
            case QUIT:
                quit = true;
                break;    
            default:
                cout << "no assigned function button" << endl;
                break;       
        }
        

        isClicked = false;
    }
};

class buttonPanel{
    
    public:    
    vector<Button> buttons;
    int buttonOffset = 40;
    int buttonWidth = 100;    

    void detectButtonClick( vector<Click> clicks, Button &button){
        Click clickTMP;
        for(int i = 0; i < clicks.size(); i++){
            clickTMP = clicks.back();

            if((clickTMP.xpos >= button.rect.x && clickTMP.xpos <= button.rect.x+button.rect.w) && 
                (clickTMP.ypos >= button.rect.y && clickTMP.ypos <= button.rect.y+button.rect.h)){
                button.isClicked = true;
            }

            clicks.pop_back();
        }
        
    }


    void buttonLayout(){
        int totalWidth = buttons.size()*buttonWidth + buttonOffset*buttons.size()-1;
        buttons[0].rect.y = 100;

        buttons[0].rect.x = SCREEN_WIDTH/2 - totalWidth/2;
        buttons[0].labelField->xStartPos = buttons[0].rect.x;
        buttons[0].labelField->yStartPos = buttons[0].rect.y;

        for(int i = 1; i < buttons.size(); i++){
            buttons[i].rect.y = buttons[0].rect.y;
            buttons[i].rect.x = buttons[i-1].rect.x + buttonWidth + buttonOffset;
            buttons[i].labelField->xStartPos = buttons[i].rect.x;
            buttons[i].labelField->yStartPos = buttons[i].rect.y;
        }


    }  

    void renderButton(Button &button){    

        //make border rect 5px bigger than button rect
        SDL_Rect buttonBorder;
        buttonBorder = button.rect;

        buttonBorder.x -= 1;
        buttonBorder.y -= 1;
        buttonBorder.h += 2;
        buttonBorder.w += 2; 

        

        if(button.isClicked){
            //set color to red
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        }

        if(!button.isClicked){
            //set color to black
            SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
        }


        
        //SDL_render fill borderrect
        SDL_RenderFillRect(renderer, &buttonBorder);        

        //SDL_setrenderdrawcolor to white
        SDL_SetRenderDrawColor( renderer, 240, 240, 240, 0xFF );
        SDL_RenderFillRect(renderer, &button.rect);

        //textField label_text(button.rect.x, button.rect.y, button.fontsize, {0, 0, 0});
        //label_text.lines.push_back( button.label);
        //label_text.render();

        button.labelField->render();
        
    } 

    void handleButtons( vector<Click> &clicks, bool &quit){
        
        for(int i = 0; i < buttons.size(); i++){
            detectButtonClick(clicks, buttons[i]);

            renderButton(buttons[i]);

            if (buttons[i].isClicked){
                buttons[i].onClick(quit); 
            }
        }
        clicks.clear();        
    }

};

class HostFrame{
    public:
    vector<textField> textfields;
    textFields activeTextField;

    buttonPanel panel;

    
    HostFrame(vector<textField> &textfields_, textFields activeTextField_){
        textfields = textfields_;
        activeTextField = activeTextField_;

    }

    void initLayout(){
        textfields[FILENAME].borderRect.h = 40;
        panel.buttonLayout(); 
    }


    void handleEvents(SDL_Event e, vector<Click> clicks, bool &quit){
        Click clickTMP;

        

        for(int i = 0; i < clicks.size(); i++){


            clickTMP = clicks.back();

            for (int j = 0; j < textfields.size(); j++)
            {
                if((clickTMP.xpos >= textfields[j].borderRect.x && clickTMP.xpos <= textfields[j].borderRect.x+textfields[j].borderRect.w) && 
                    (clickTMP.ypos >= textfields[j].borderRect.y && clickTMP.ypos <= textfields[j].borderRect.y+textfields[j].borderRect.h)){
                        activeTextField = (textFields)j;
                    }
            }

            clicks.pop_back();
        }

        

        switch(activeTextField){
            case FILENAME:
                textfields[FILENAME].handleEvent(e);
                break;
            case TEXTFIELD:
                textfields[TEXTFIELD].handleEvent(e);
                break;
        }

        textfields[FILENAME].assignedFile->name = textfields[FILENAME].lines[0];
        textfields[TEXTFIELD].assignedFile->name = textfields[FILENAME].lines[0];
    }

    void render(){
        for (int i = 0; i < textfields.size(); i++){
            textfields[i].render();
        }
    }
};



/*
problem - needs better encapsulation, panel button handling should be
        called by hostFrame not directly by main, more class parameters should be private

    solution:
        ---   

problem - cursor position doesn't account for different letter width

    solution:
        (lazy one) pick a uniform letter width font :D

problem - deleting at the end of the line, not at cursor position
        (to be fixed AFTER the cursor position)

    solution:
        ---

*/


int main(int, char**) {
    SDL_Init(SDL_INIT_VIDEO);    	
    TTF_Init();

    vector<Click> clicks;
    
    IOfile file ("text.txt");

    vector<textField> textfields;
    HostFrame hostFrame(textfields, TEXTFIELD); 

    hostFrame.textfields.push_back(textField(10, 10, 20, {0, 0, 0}, &file));
    hostFrame.textfields[FILENAME].lines.push_back("text.txt");
    hostFrame.textfields[FILENAME].isFileChanger = 1;    
    hostFrame.textfields.push_back(textField(10, 200, 25, {0, 0, 0}, &file));    
    file.openAndRead(hostFrame.textfields[TEXTFIELD].lines);
    
    hostFrame.panel.buttons.push_back(Button("read", READ, &hostFrame.textfields[TEXTFIELD]));    
    hostFrame.panel.buttons.push_back(Button ("write", WRITE, &hostFrame.textfields[TEXTFIELD]));    
    hostFrame.panel.buttons.push_back(Button ("quit", QUIT));

    hostFrame.initLayout();

    bool quit = false;
    SDL_Event e;

    Click clickTMP;

    while(!quit){
        while(SDL_PollEvent(&e) != 0){
            if( e.type == SDL_QUIT ){
                quit = true;
            }

            else if (e.type==SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT ){
                SDL_GetMouseState(&clickTMP.xpos, &clickTMP.ypos);
                clicks.push_back(clickTMP);
            }

            hostFrame.handleEvents(e, clicks, quit);
            
        }
        
        SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear( renderer );


        hostFrame.panel.handleButtons(clicks, quit);  //!     

        hostFrame.render();


        SDL_RenderPresent( renderer );
        SDL_Delay(50);

    }

    TTF_Quit();
    SDL_Quit();

    
}
