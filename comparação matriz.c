#include<stdio.h>
#include<time.h>
#include<stdlib.h>
//
// lembrar de desativar a otimização do gcc -O0 
//
//Tempo da compara_1 (Linhas): 0.147212 segundos
//Tempo da compara_2 (Colunas): 0.241707 segundos
//Diferenca de performance: 1.64x
//
//
//

#define N 10000

int A[N][N];

void compara_1 (){
    long long soma = 0;
    for(int i=0;i<N;i++)
        for(int j=0; j<N; j++)
            soma += A[i][j];
}

void compara_2(){
    long long soma = 0;
    for(int j=0;j<N;j++)
        for(int i=0; i<N; i++)
            soma += A[i][j];
}



int main() {
    clock_t inicio, fim;
    double tempo_1, tempo_2;

    // Medindo a Função 1 (Percorre por Linhas)
    inicio = clock();
    compara_1();
    fim = clock();
    tempo_1 = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    // Medindo a Função 2 (Percorre por Colunas)
    inicio = clock();
    compara_2();
    fim = clock();
    tempo_2 = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    printf("Tempo da compara_1 (Linhas): %f segundos\n", tempo_1);
    printf("Tempo da compara_2 (Colunas): %f segundos\n", tempo_2);
    printf("Diferenca de performance: %.2fx\n", tempo_2 / tempo_1);

    return 0;
}