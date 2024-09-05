// 编译时需要链接 winmm.dll: 
// gcc videoPlay.c -o videoPlay.exe -lwinmm

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>

#define FRAME_WIDTH 150
#define FRAME_HEIGHT 38
#define FRAME_COUNT 10152
#define FPS 24

#define FRAME_DATA_PATH "frames.data"  // 帧数据文件位置
#define AUDIO_DATA_PATH "audio.mp3"  // 音频文件位置
#define CHARACTER "#"   // 在控制台用来显示图片的字符

typedef struct Pixel {
    unsigned char B,G,R;
} Pixel;

typedef struct Frame {
    struct Pixel pixels[FRAME_WIDTH * FRAME_HEIGHT];
} Frame;


Frame *frames = NULL; // 视频所有帧的数据



// 开启Windows的虚拟终端序列支持
int enableVTMode() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return -1;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        return -2;

    dwMode |= 0x0004;
    if (!SetConsoleMode(hOut, dwMode))
        return -3;
    return 1;
}



// 读取帧数据
int readFramesData() {
    FILE *file = fopen(FRAME_DATA_PATH, "rb");
    if(file == NULL)
        return -1;

    frames = malloc(sizeof(Frame) * FRAME_COUNT);
    fread(frames, sizeof(Frame), FRAME_COUNT, file);
    fclose(file);

    return 0;
}



// 打印一帧的内容
char string_buffer[(FRAME_WIDTH+1) * (FRAME_HEIGHT+1) * 24];

void displayFrame(Frame frame) {
    printf("\033[H");
    int buffer_index = 0;
    for(int y = 0; y < FRAME_HEIGHT; ++y) {
        for(int x = 0; x < FRAME_WIDTH; ++x) {
            Pixel pixel = frame.pixels[x + y*FRAME_WIDTH];  // 获取图片中 (x, y) 点的像素值
            buffer_index += sprintf(string_buffer + buffer_index,"\033[38;2;%d;%d;%dm" CHARACTER, pixel.R, pixel.G, pixel.B);
        }
        buffer_index += sprintf(string_buffer + buffer_index,"\n");
    }
    string_buffer[buffer_index] = '\0';
    printf(string_buffer);
    fflush(stdout);
}



// 调用Windows的API播放音频
void playSound(){
    char buff[255], command[40];
    sprintf(command, "open " AUDIO_DATA_PATH " alias playsound_134");
    mciSendStringA(command, buff, 254, NULL);
    sprintf(command, "set playsound_134 time format milliseconds");
    mciSendStringA(command, buff, 254, NULL);
    sprintf(command, "status playsound_134 length");
    mciSendStringA(command, buff, 254, NULL);
    sprintf(command, "play playsound_134 from 0 to %s",buff);
    mciSendStringA(command, buff, 254, NULL);
}



// 播放视频
void playVideo() {

    LARGE_INTEGER cpuFrequency = { 0 };  //CPU时钟频率
    QueryPerformanceFrequency(&cpuFrequency);  //获取CPU时钟频率

    LARGE_INTEGER start_time,time1,time2;

    QueryPerformanceCounter(&start_time); // 获取当前CPU时钟计数

    for(int i = 0; i < FRAME_COUNT; ){
        QueryPerformanceCounter(&time1);  
        displayFrame(frames[i]);
        QueryPerformanceCounter(&time2);
        double frame_time = ((double)time2.QuadPart - (double)time1.QuadPart) / (double)cpuFrequency.QuadPart;
        
        if (i % FPS == 0)
            printf("\033[0mfps: %3.2f,  frame: %d/%d", 1 / frame_time, i, FRAME_COUNT);
        
        double time_since_start = ((double)time2.QuadPart - (double)start_time.QuadPart) / (double)cpuFrequency.QuadPart;
        i = (int)(FPS * time_since_start);
    }
}



int main(int argc, char *argv[]){

    // 设置控制台缓冲区大小
    setvbuf(stdout, NULL, _IOFBF, (FRAME_WIDTH+1) * (FRAME_HEIGHT+1) * 24);

    // 开启Windows的虚拟终端序列支持
    if(enableVTMode() < 0){
        printf("终端不支持ANSI转义序列.");
        return -1;
    }

    // 打开数据文件
    if(readFramesData() < 0){
        printf("文件 '" FRAME_DATA_PATH "' 打开失败！");
        return -1;
    }

    system("mode con cols=200 lines=50");  // 设置控制台大小
    system("cls");  // 控制台清屏
    printf("\033[?25l");  // 隐藏光标

    playSound();
    playVideo();

    // 清除文字样式
    printf("\033[0m");

    free(frames);

    system("pause");
    return 0;
}