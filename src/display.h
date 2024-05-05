
#ifndef DISPLAY_H_
#define DISPLAY_H_

void setup_display();

void show_display(String header, int wait = 0);
void show_display(String header, String line1, int wait = 0);
void show_display(String header, String line1, String line2, int wait = 0);
void show_display(String header, String line1, String line2, String line3, int wait = 0);
void show_display(String header, String line1, String line2, String line3, String line4, int wait = 0);
void show_display(String header, String line1, String line2, String line3, String line4, String line5, int wait = 0);
void display_7lines(String line0, String line1, String line2, String line3, String line4, String line5, String line6,int wait = 0);


#endif
