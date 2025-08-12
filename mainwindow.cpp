#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QClipboard>
#include <algorithm>
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <cmath>
#define TOLOWER(p) {transform(p.begin(),p.end(),p.begin(),::tolower);}  //转小写awa
#define ll long long
#define con 1100000
using namespace std;

unsigned char strInput[con], keyInput[con], textChiper[con];
unsigned char strHex16Part[16], keyHex16Part[16], strHex32Part[32], keyHex32Part[32];
int strInputLen, keyInputLen, str[con], key[con];
int mode = 1, strHexCnt, miaoMode = 2;
char strEnOut[con], strDeOut[con];
string strEncrypted, miao[] = {u8"", u8"喵", u8"嘿", u8"呜", u8"嗷"};
QString qMiao[] = {u8"", u8"喵", u8"嘿", u8"呜", u8"嗷"};

// ============================== 初始化awa ==============================
//获取unsigned char的长度awa
int MyStrlen(unsigned char* str)
{
    unsigned char* temp = str;
    while('\0' != *str) str++;
    return str-temp;
}
//字面意思awa
string QStrToStr(const QString qStr)
{
    QByteArray data = qStr.toLocal8Bit();
    string localStr = data.data();
    return localStr;
}

// 专用控制台输出函数awa
void PrintUTF8(const std::string& str) {
    SetConsoleOutputCP(65001); // 切换到UTF-8模式
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleA(hConsole, str.c_str(), str.size(), NULL, NULL);
}

void init() {
    strEncrypted = "";
    memset(str, 0, sizeof str);
    memset(key, 0, sizeof key);
    memset(strEnOut, 0, sizeof strEnOut);
    memset(strDeOut, 0, sizeof strDeOut);
    memset(textChiper, 0, sizeof textChiper);
    memset(strHex16Part, 0, sizeof strHex16Part);
    memset(strHex32Part, 0, sizeof strHex32Part);
    memset(keyHex16Part, 0, sizeof keyHex16Part);
    memset(keyHex32Part, 0, sizeof keyHex32Part);

    //获取原始长度awa
    strInputLen=MyStrlen(strInput);
    keyInputLen=MyStrlen(keyInput);

    //补齐strInput和keyInput大小为16的倍数awa
    strInput[strInputLen] = (char)127;
    strInputLen++;
    while(strInputLen % 16 != 0) {
        strInput[strInputLen] = '0';
        strInputLen++;
    }

    //填充key awa
    if(keyInputLen<strInputLen && keyInputLen != 0) {
        int i=0;
        for(; strInputLen>keyInputLen; keyInputLen++) {
            keyInput[keyInputLen] = keyInput[i++];
        }
    } else {
        for(; strInputLen>keyInputLen; keyInputLen++) {
            keyInput[keyInputLen] = '0';
        }
    }

    //将ASCII码存入数组awa
    for(int i=0; i<strInputLen; i++) {
        str[i]=strInput[i];
        if(str[i]<0) {
            str[i] += 256;
        }
    }
    for(int i=0; i<keyInputLen; i++) {
        key[i]=keyInput[i];
        if(key[i]<0) {
            key[i] += 256;
        }
    }
    return ;
}

// ============================== 十进制转十六进制awa ==============================
string DecToHex(ll num)
{
    string strHex;
    ll Temp = num / 16;
    ll left = num % 16;
    if (Temp > 0)
        strHex += DecToHex(Temp);
    if (left < 10)
        strHex += (left + '0');
    else
        strHex += ('A' + left - 10);

    return strHex;
}

// ============================== 十六进制转十进制awa ==============================
ll HexToDec(string str)
{
    ll iDec = 0;
    TOLOWER(str);
    sscanf_s(str.c_str(), "%x", &iDec);
    return iDec;
}

// ============================== AES核心常量与函数awa ==============================

// AES S盒（加密用）awa
const unsigned char sbox[256]={
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

// 逆S盒（解密用）awa
const unsigned char inv_sbox[256]={
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

// 轮常量（用于密钥扩展）
const unsigned char Rcon[11]={
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
};

// ============================== 密钥扩展函数awa ==============================

// 密钥扩展函数awa
void KeyExpansion(const unsigned char* key, unsigned char* roundKeys) {
    // 初始密钥直接复制
    memcpy(roundKeys, key, 16);

    for (int i=4; i<44; i++) {
        unsigned char temp[4];
        memcpy(temp, roundKeys + (i-1)*4, 4);

        if (i % 4 == 0) {
            // 循环左移
            unsigned char t=temp[0];
            temp[0]=temp[1];
            temp[1]=temp[2];
            temp[2]=temp[3];
            temp[3]=t;

            // S盒替换
            for (int j=0; j<4; j++) {
                temp[j]=sbox[temp[j]];
            }

            // 异或轮常量
            temp[0] ^= Rcon[i/4];
        }

        // 生成新密钥
        for (int j=0; j<4; j++) {
            roundKeys[i*4 + j]=roundKeys[(i-4)*4 + j] ^ temp[j];
        }
    }
    return ;
}

// ============================== 加密操作awa ==============================

// 字节替换（加密）awa
void SubBytes(unsigned char state[16]) {
    for (int i=0; i<16; i++) {
        state[i]=sbox[state[i]];
    }
    return ;
}

// 行移位（加密）awa
void ShiftRows(unsigned char state[16]) {
    unsigned char temp[16];

    // 第0行不移位
    temp[0]=state[0];  temp[4]=state[4];
    temp[8]=state[8];  temp[12]=state[12];

    // 第1行左移1字节
    temp[1]=state[5];  temp[5]=state[9];
    temp[9]=state[13]; temp[13]=state[1];

    // 第2行左移2字节
    temp[2]=state[10]; temp[6]=state[14];
    temp[10]=state[2]; temp[14]=state[6];

    // 第3行左移3字节
    temp[3]=state[15]; temp[7]=state[3];
    temp[11]=state[7]; temp[15]=state[11];

    memcpy(state, temp, 16);
    return ;
}

// 伽罗瓦域乘法（有限域GF(2^8))awa
unsigned char gmul(unsigned char a, unsigned char b) {
    unsigned char p=0;
    for (int i=0; i<8; i++) {
        if (b & 1) p ^= a;
        bool carry=a & 0x80;
        a <<= 1;
        if (carry) a ^= 0x1B;  // 模x^8 + x^4 + x^3 + x + 1
        b >>= 1;
    }
    return p;
}

// 列混淆（加密）awa
void MixColumns(unsigned char state[16]) {
    unsigned char temp[16];
    for (int i=0; i<4; i++) {
        temp[4*i+0]=gmul(0x02, state[4*i+0]) ^
                          gmul(0x03, state[4*i+1]) ^
                          state[4*i+2] ^
                          state[4*i+3];

        temp[4*i+1]=state[4*i+0] ^
                          gmul(0x02, state[4*i+1]) ^
                          gmul(0x03, state[4*i+2]) ^
                          state[4*i+3];

        temp[4*i+2]=state[4*i+0] ^
                          state[4*i+1] ^
                          gmul(0x02, state[4*i+2]) ^
                          gmul(0x03, state[4*i+3]);

        temp[4*i+3]=gmul(0x03, state[4*i+0]) ^
                          state[4*i+1] ^
                          state[4*i+2] ^
                          gmul(0x02, state[4*i+3]);
    }
    memcpy(state, temp, 16);
    return ;
}

// 轮密钥加awa
void AddRoundKey(unsigned char state[16], const unsigned char* roundKey) {
    for (int i=0; i<16; i++) {
        state[i] ^= roundKey[i];
    }
    return ;
}

// ============================== 解密操作awa ==============================

// 逆向字节替换（解密）awa
void InvSubBytes(unsigned char state[16]) {
    for (int i=0; i<16; i++) {
        state[i]=inv_sbox[state[i]];
    }
    return ;
}

// 逆向行移位（解密）awa
void InvShiftRows(unsigned char state[16]) {
    unsigned char temp[16];

    // 第0行不移位awa
    temp[0]=state[0];  temp[4]=state[4];
    temp[8]=state[8];  temp[12]=state[12];

    // 第1行右移1字节awa
    temp[5]=state[1];  temp[9]=state[5];
    temp[13]=state[9]; temp[1]=state[13];

    // 第2行右移2字节awa
    temp[10]=state[2]; temp[14]=state[6];
    temp[2]=state[10]; temp[6]=state[14];

    // 第3行右移3字节awa
    temp[15]=state[3]; temp[3]=state[7];
    temp[7]=state[11]; temp[11]=state[15];

    memcpy(state, temp, 16);
    return ;
}

// 逆向列混淆（解密）awa
void InvMixColumns(unsigned char state[16]) {
    unsigned char temp[16];
    for (int i=0; i<4; i++) {
        temp[4*i+0]=gmul(0x0e, state[4*i+0]) ^
                          gmul(0x0b, state[4*i+1]) ^
                          gmul(0x0d, state[4*i+2]) ^
                          gmul(0x09, state[4*i+3]);

        temp[4*i+1]=gmul(0x09, state[4*i+0]) ^
                          gmul(0x0e, state[4*i+1]) ^
                          gmul(0x0b, state[4*i+2]) ^
                          gmul(0x0d, state[4*i+3]);

        temp[4*i+2]=gmul(0x0d, state[4*i+0]) ^
                          gmul(0x09, state[4*i+1]) ^
                          gmul(0x0e, state[4*i+2]) ^
                          gmul(0x0b, state[4*i+3]);

        temp[4*i+3]=gmul(0x0b, state[4*i+0]) ^
                          gmul(0x0d, state[4*i+1]) ^
                          gmul(0x09, state[4*i+2]) ^
                          gmul(0x0e, state[4*i+3]);
    }
    memcpy(state, temp, 16);
    return ;
}

// ============================== AES主函数awa ==============================

// AES加密主函数awa
void AES_encrypt(const unsigned char* plain,
                 const unsigned char* key,
                 unsigned char* cipher) {
    // 1. 密钥扩展（生成11个轮密钥）awa
    unsigned char roundKeys[176];
    KeyExpansion(key, roundKeys);

    // 2. 初始化状态矩阵awa
    unsigned char state[16];
    memcpy(state, plain, 16);

    // 3. 初始轮（仅轮密钥加）awa
    AddRoundKey(state, roundKeys);

    // 4. 主循环（9轮）awa
    for (int round=1; round <= 9; round++) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, roundKeys + round*16);
    }

    // 5. 最终轮（无列混淆）awa
    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(state, roundKeys + 10*16);

    // 输出密文awa
    memcpy(cipher, state, 16);
    return ;
}

// AES解密主函数awa
void AES_decrypt(const unsigned char* cipher,
                 const unsigned char* key,
                 unsigned char* plain) {
    // 1. 密钥扩展（生成11个轮密钥）awa
    unsigned char roundKeys[176];
    KeyExpansion(key, roundKeys);

    // 2. 初始化状态矩阵awa
    unsigned char state[16];
    memcpy(state, cipher, 16);

    // 3. 初始轮（使用最后一轮密钥）awa
    AddRoundKey(state, roundKeys + 10*16);

    // 4. 主循环（9轮）awa
    for (int round=9; round >= 1; round--) {
        InvShiftRows(state);
        InvSubBytes(state);
        AddRoundKey(state, roundKeys + round*16);
        InvMixColumns(state);
    }

    // 5. 最终轮awa
    InvShiftRows(state);
    InvSubBytes(state);
    AddRoundKey(state, roundKeys);

    // 输出明文awa
    memcpy(plain, state, 16);
    return ;
}

// ============================== 辅助函数awa ==============================

// 打印十六进制数据awa
void PrintHex(const unsigned char* data, int len, const string& tag) {
    cout<<tag<<": ";
    for (int i=0; i<len; i++) {
        cout<<hex<<setw(2)<<setfill('0')
             << static_cast<int>(data[i])<<" ";
        //		cout<<(int)(data[i])<<' ';
    }
    cout<<dec<<endl;
    return ;
}

void SaveStrEncrypted(const unsigned char* data, int len) {
    for (int i=0; i<len; i++) {
        string tmp = DecToHex((int)data[i]);
        //对于单字节16进制补0
        if(tmp.length()==1) {
            tmp.insert(0, "0");
        }
        strEncrypted += tmp;
        //cout<<DecToHex((int)data[i])<<' ';
    }
    return ;
}

// 从十六进制字符串转换为字节数组（带验证）awa
void HexStringToBytes(const string& hex, unsigned char* bytes, int len) {
    // 验证输入长度awa
    if (hex.length() != static_cast<size_t>(len*2)) {
        throw runtime_error("输入的十六进制字符串长度不正确");
    }

    for (int i=0; i<len; i++) {
        string byteString=hex.substr(i*2, 2);
        //		cout<<byteString<<' ';

        // 验证是否为有效的十六进制字符awa
        if (!isxdigit(byteString[0]) || !isxdigit(byteString[1])) {
            throw runtime_error("包含非十六进制字符: " + byteString);
        }

        // 安全转换awa
        try {
            bytes[i]=static_cast<unsigned char>(stoul(byteString, nullptr, 16));
        } catch (const exception& e) {
            throw runtime_error("转换失败: " + byteString + " - " + e.what());
        }
    }
    return ;
}

// 输入十六进制数据（带错误处理）awa
void InputHexData(unsigned char* text, unsigned char* data, int len) {
    string input = (reinterpret_cast<const char*>(text));
    input.erase(input.begin()+32, input.end());
    cout<<input<<'\n';
    while (true) {
        //getline(cin, input);
        // 移除所有空格awa
        input.erase(remove(input.begin(), input.end(), ' '), input.end());

        // 转换为大写以便统一处理awa
        transform(input.begin(), input.end(), input.begin(), ::toupper);

        try {
            HexStringToBytes(input, data, len);
            return ;
        } catch (const exception& e) {
            cout<<"错误: "<<e.what()<<endl;
            cout<<"请输入"<<len*2<<"个有效的十六进制字符（0-9, A-F），例如";
            for (int i=0; i<len; i++) cout<<"A0";
            cout<<"\n";
            return ;
        }
    }
    return ;
}

// 打印AES状态矩阵awa
//void PrintState(const unsigned char* state, const string& title) {
//	cout<<"\n"<<title<<":\n";
//	for (int i=0; i<4; i++) {
//		for (int j=0; j<4; j++) {
//			cout<<hex<<setw(2)<<setfill('0')
//			<< static_cast<int>(state[i + j*4])<<" ";
//		}
//		cout<<endl;
//	}
//	cout<<dec;
//	return ;
//}

bool CopyToClipboard(const QString& text) {
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);
    return true;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_label_input_linkActivated(const QString &link)
{

}

void MainWindow::on_textEdit_strInput_textChanged()
{

}

void MainWindow::on_textEdit_keyInput_textChanged()
{

}

void MainWindow::on_textEdit_strOutput_textChanged()
{

}

void MainWindow::on_CatLanModeChange_clicked()
{
    if(miaoMode == 1) {
        miaoMode = 2;
        ui -> CatLanModeChange -> setText("喵喵喵?：关awa");
    } else {
        miaoMode = 1;
        ui -> CatLanModeChange -> setText("喵喵喵?：开awa");
    }
}

void MainWindow::on_modeChange_clicked()
{
    if(mode == 1) {
        mode = 2;
        ui -> modeChange ->setText("当前模式：解密awa");
        ui -> label_input -> setText("在此输入密文awa");
        ui -> CatLanModeChange -> setEnabled(false);
    } else {
        mode = 1;
        ui -> modeChange ->setText("当前模式：加密awa");
        ui -> label_input -> setText("在此输入明文awa");
        ui -> CatLanModeChange -> setEnabled(true);
    }
}

void MainWindow::on_goWork_clicked()
{
    memset(strInput, 0, sizeof strInput);
    memset(keyInput, 0, sizeof keyInput);
    //输入awa
    QString qStrInput = ui -> textEdit_strInput -> toPlainText();
    QString qKeyInput = ui -> textEdit_keyInput -> toPlainText();
    // QString qStrInput = "呜喵嗷嘿喵喵喵嗷";
    string sStrInput = QStrToStr(qStrInput);
    string sKeyInput = QStrToStr(qKeyInput);
    //喵喵喵？awa
    for(int i = 1; i <= 4; i++) {
        string byteString = sStrInput.substr(0, 2);
        string miaoTmp = QStrToStr(qMiao[i]);
        //cout << "byteString: " << byteString << "   miao[i]: " << miao[i] << '\n';
        // 调试输出：显示原始HEX值awa
        cout << "byteString hex: ";
        for(char c : byteString) printf("%02X ", (uint8_t)c);
        cout << "miaoTmp: ";
        for(char c : miaoTmp) printf("%02X ", (uint8_t)c);

        if(byteString == miaoTmp) {
            cout << "start miao" << '\n';

            string strMiaoToHex = "";
            int chNum = 0, sStrInputLen = sStrInput.length();

            for(int j=0; j<sStrInputLen/2; j++) {
                byteString = sStrInput.substr(j*2, 2);
                for(int k=1; k<=3; k++) {
                    miaoTmp = QStrToStr(qMiao[k]);
                    if(byteString == miaoTmp) {
                        chNum += 4 * (k-1);
                        if(k==1) chNum++;
                        break;
                    }
                }

                if(byteString == QStrToStr(qMiao[4])) {
                    strMiaoToHex += DecToHex(chNum);
                    cout<<"strMiaoToHex: "<<strMiaoToHex<<'\n';
                    chNum = 0;
                }

            }
            sStrInput = strMiaoToHex;
            break;
        } else {
            cout << "quick miao" << '\n';
        }
    }
    // memcpy(strInput, sStrInput.data(), sStrInput.size());
    // memcpy(keyInput, sKeyInput.data(), sKeyInput.size());
    // QString qqStrInput = QString::fromStdString(qStrInput);
    // ui -> textEdit_strOutput -> setText(qqStrInput);
    for (int i = 0; i < sStrInput.size(); i++) {
        strInput[i] = static_cast<unsigned char>(sStrInput[i]);
    }
    for (int i = 0; i < sKeyInput.size(); i++) {
        keyInput[i] = static_cast<unsigned char>(sKeyInput[i]);
    }

    string tmpStrInput = (char*)strInput;
    cout<<"strInput:"<<sStrInput<<'\n';


    init();
    memset(keyInput, 0, sizeof keyInput);
    if(mode==1) {
        //加密awa
        //将明文和密钥以16进制存入待加密的数组awa
        memset(strInput, 0, sizeof strInput);
        strHexCnt = 0;

        for(int i=0; i<strInputLen; i++) {
            string strHex = DecToHex(str[i]);
            //对于单字节16进制补0 awa
            if(strHex.length()==1) {
                strHex.insert(0, "0");
            }
            for(int i=0; i<2; i++) {
                strInput[strHexCnt] = strHex[i];
                strHexCnt++;
            }
        }

        strInputLen *= 2;
        strHexCnt = 0;

        for(int i=0; i<keyInputLen; i++) {
            string tmp = DecToHex(key[i]);
            for(int i=0; i<2; i++) {
                keyInput[strHexCnt] = tmp[i];
                strHexCnt++;
            }
        }

        keyInputLen *= 2;
        strHexCnt = 0;

        //测试函数awa
        // cout<<"str["<<strInputLen<<"]: ";
        // for(int i=0; i<strInputLen; i++) {
        //     cout<<strInput[i]<<' ';
        // }
        // cout<<'\n'<<"key["<<keyInputLen<<"]: ";
        // for(int i=0; i<keyInputLen; i++) {
        //     cout<<keyInput[i]<<' ';
        // }

        for(int i=1; i<=strInputLen/32; i++) {
            memset(strHex32Part, 0, sizeof strHex32Part);
            memcpy(strHex32Part, strInput + strHexCnt*32, 32);
            memset(keyHex32Part, 0, sizeof keyHex32Part);
            memcpy(keyHex32Part, keyInput + strHexCnt*32, 32);
            strHexCnt++;

            printf("Input:     ");
            InputHexData(strHex32Part, strHex16Part, 16);
            printf("Key:     ");
            InputHexData(keyHex32Part, keyHex16Part, 16);

            AES_encrypt(strHex16Part, keyHex16Part, textChiper);
            //PrintHex(textChiper, 16, "密文      ");
            SaveStrEncrypted(textChiper, 16);
        }
        cout<<"EnEnd:     "<<strEncrypted<<'\n';

        //喵喵喵?awa
        if(miaoMode == 1) {
            string strEnTmp = "";
            for(int i=0; i<strEncrypted.length(); i++) {
                string strTmp = strEncrypted.substr(i, 1);
                int strDec = HexToDec(strTmp);
                if(strDec > 0) {
                    if(strDec >= 8) {
                        int str8cnt = strDec / 8;
                        strDec %= 8;
                        while(str8cnt--) {
                            strEnTmp += miao[3];
                        }
                    }
                    if(strDec >= 4) {
                        strDec -= 4;
                        strEnTmp += miao[2];
                    }
                    if(strDec >= 1) {
                        while(strDec--) {
                            strEnTmp += miao[1];
                        }
                    }
                }
                strEnTmp += miao[4];
            }
            strEncrypted = strEnTmp;
            cout<<"EnMiaoEnd:     "<<strEncrypted<<'\n';
        }

        QString qStrEncrypted = QString(QString::fromUtf8(strEncrypted.data()));
        ui -> textEdit_strOutput -> setText(qStrEncrypted);

        if(CopyToClipboard(qStrEncrypted)) {
            ui -> goWork -> setText("结果已复制到剪切板中awa");
        } else {
            ui -> goWork -> setText("处理成功awa");
        }


    } else if(mode==2) {
        //解密awa
        //将密钥以16进制存入待加密的数组awa
        strHexCnt = 0;
        for(int i=0; i<keyInputLen; i++) {
            string tmp = DecToHex(key[i]);
            for(int i=0; i<2; i++) {
                keyInput[strHexCnt] = tmp[i];
                strHexCnt++;
            }
        }

        keyInputLen *= 2;
        strHexCnt = 0;

        for(int i=1; i<=strInputLen/32; i++) {
            memset(strHex32Part, 0, sizeof strHex32Part);
            memcpy(strHex32Part, strInput + strHexCnt*32, 32);
            memset(keyHex32Part, 0, sizeof keyHex32Part);
            memcpy(keyHex32Part, keyInput + strHexCnt*32, 32);
            strHexCnt++;

            printf("input:     ");
            InputHexData(strHex32Part, strHex16Part, 16);
            printf("key:     ");
            InputHexData(keyHex32Part, keyHex16Part, 16);

            AES_decrypt(strHex16Part, keyHex16Part, textChiper);
            //PrintHex(textChiper, 16, "密文      ");
            SaveStrEncrypted(textChiper, 16);
        }

        //cout<<"Hex:     "<<strEncrypted<<'\n';
        //去除后缀0 awa
        string strEnLast = strEncrypted.substr(strEncrypted.length()-2, 2);
        while(strEnLast == "30") {
            strEncrypted.erase(strEncrypted.end()-2, strEncrypted.end());
            strEnLast = strEncrypted.substr(strEncrypted.length()-2, 2);
        }
        if(strEnLast == "7F") {
            strEncrypted.erase(strEncrypted.end()-2, strEncrypted.end());
        }
        cout<<"Hex(no 0):   "<<strEncrypted<<'\n';

        //将16进制明文转换为人可以看懂的明文awa
        for(int i=0; i<=strEncrypted.length()/2; i++) {
            string byteString=strEncrypted.substr(i*2, 2);
            strDeOut[i] = (char)HexToDec(byteString);
            if(strDeOut[i] <= 0) {
                strDeOut[i] += 256;
            }
        }

        //cout<<"DeEnd:     ";
        string stringDeOut = "";
        // for(int i=0; i<=strEncrypted.length()/2; i++) {
        //     //cout<<strEnOut[i];
        // }
        stringDeOut = strDeOut;
        cout<<stringDeOut<<'\n';

        QString qStrDecrypted = QString(QString::fromLocal8Bit(stringDeOut.data()));
        ui -> textEdit_strOutput -> setText(qStrDecrypted);

        if(CopyToClipboard(qStrDecrypted)) {
            ui -> goWork -> setText("结果已复制到剪切板中awa");
        } else {
            ui -> goWork -> setText("处理成功awa");
        }
    }
}
//完结撒花awa!!



/*

  code by
      Xflinphyz

*/

