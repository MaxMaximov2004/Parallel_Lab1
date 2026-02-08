// ConsoleApplication3.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <string>
#include <math.h>

// Data
struct data {
public:
    std::vector<float> x;
    std::vector<float> y;
    float step;
    data(int N) {
        step = 0;
        x.resize(N,0.0);
        y.resize(N,0.0);
    }

    // Вспомогательные методы для SIMD
    float* y_data() { return y.data(); }
    size_t y_size() const { return y.size(); }
};

data generate(float x_1, float x_n, int count_n, float (*funct)(float)) {

    data intgrl_data(count_n+1);
    
    float step = (x_n - x_1) / count_n;
    float x = x_1;
    intgrl_data.step = step;

    for(int i = 0; i<=count_n; i++)
    {
        intgrl_data.x[i] = x;
        intgrl_data.y[i] = funct(x);
        x += step;
    }

    return intgrl_data;
}

std::string vis_data(data Data) 
{
    std::string str = "Y:\t";

    for (float& y : Data.y) {
        str += std::to_string(y) + " ";
    }
    str += "\nX:\t";

    for (float& x : Data.x) {
        str += std::to_string(x) + " ";
    }

    return str;
}


//Algorithms
double Left_rect(data intgrl_data) {
    double integral = 0.0;
    
    for (int i = 0; i < (intgrl_data.x.size() - 1); i++) {
        integral += intgrl_data.y[i] * intgrl_data.step;
    }

    return integral;
}

double Right_rect(data intgrl_data) {
    double integral = 0.0;

    for (int i = 0; i < (intgrl_data.x.size() - 1); i++) {
        integral += intgrl_data.y[i+1] * intgrl_data.step;
    }

    return integral;
}

double Simpson(data intgrl_data) {
    double integral = intgrl_data.y[0]+intgrl_data.y[intgrl_data.y.size()-1];

    for (int i = 2; i < intgrl_data.y.size() - 1; i+=2) {
        integral += 2 * intgrl_data.y[i];
    }

    for (int i = 1; i < intgrl_data.y.size() - 2; i += 2) {
        integral += 4 * intgrl_data.y[i];
    }

    return (integral * intgrl_data.step) / 3;
}

//Assembler algorithms
// РАБОТАЕТ ТОЛЬКО В 32-БИТНОМ РЕЖИМЕ (x86)!
// Проект → Свойства → Configuration Properties → General → Platform Toolset → x86

double Left_rect_ASM(data intgrl_data) {
    double integral = 0.0;
    size_t n = intgrl_data.y_size() - 1;
    float step = intgrl_data.step;
    float* y = intgrl_data.y_data();

    __asm {
        mov esi, y          // esi = указатель на y[0]
        mov ecx, n          // ecx = счётчик
        xorps xmm0, xmm0    // xmm0 = накопитель суммы
        movss xmm1, step    // xmm1 = step
        shufps xmm1, xmm1, 0 // размножить step на все 4 компонента

        loop_start:
        cmp ecx, 4
            jl short tail       // если < 4 элементов — переход к хвосту

            // Загрузить 4 значения, умножить на step, добавить к сумме
            movups xmm2, [esi]  // xmm2 = y[i], y[i+1], y[i+2], y[i+3]
            mulps xmm2, xmm1    // xmm2 *= step
            addps xmm0, xmm2    // xmm0 += xmm2

            add esi, 16         // сдвинуть указатель на 4 float (16 байт)
            sub ecx, 4
            jmp loop_start

            tail :
        test ecx, ecx
            jz short done

            tail_loop :
        movss xmm2, [esi]
            mulss xmm2, xmm1
            addss xmm0, xmm2
            add esi, 4
            dec ecx
            jnz tail_loop

            done :
        // Сложить 4 компонента xmm0 в одно значение
        movhlps xmm1, xmm0
            addps xmm0, xmm1
            movaps xmm1, xmm0
            shufps xmm1, xmm1, 1
            addss xmm0, xmm1

            // Сохранить результат во временную переменную
            movss dword ptr integral, xmm0
    }

    return integral;
}
int main()
{
    auto Funct{ [](float x)->float {return x * x; } };
    data Data = generate(0, 20, 10000, Funct);
    //std::cout << vis_data(Data)<<std::endl;
    std::cout << "Classic: " << std::endl;
    std::cout << Left_rect(Data) << std::endl;
    std::cout << Right_rect(Data) << std::endl;
    std::cout << Simpson(Data) << std::endl;
    std::cout << "Assembler: " << std::endl;
    std::cout << Left_rect_ASM(Data) << std::endl;


}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
