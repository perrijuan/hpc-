/*
 * gemm_orders.c
 * Comparação das 6 ordenações de loops para C := AB + C
 * Baseado no material "Loops and More Loops" (UT Austin / FLAME)
 *
 * Compilar com:
 *   gcc -O3 -march=native -o gemm_orders gemm_orders.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define min(a,b) ((a) < (b) ? (a) : (b))

// Macros para acesso column-major (como no PDF)
#define alpha(i,j) A[(j)*ldA + (i)]
#define beta(i,j)  B[(j)*ldB + (i)]
#define gamma(i,j) C[(j)*ldC + (i)]

// Função para medir tempo com alta precisão
double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

// Inicializa matriz com valores aleatórios
void init_matrix(double *M, int rows, int cols, int ld) {
    for (int j = 0; j < cols; j++) {
        for (int i = 0; i < rows; i++) {
            M[j*ld + i] = (double)rand() / RAND_MAX * 2.0 - 1.0; // [-1, 1]
        }
    }
}

// Zera matriz C
void zero_matrix(double *C, int m, int n, int ldC) {
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < m; i++) {
            C[j*ldC + i] = 0.0;
        }
    }
}

// ============================================================
// AS 6 ORDENAÇÕES DE LOOPS
// ============================================================

// 1. IJP
void gemm_IJP(int m, int n, int k,
              double *A, int ldA,
              double *B, int ldB,
              double *C, int ldC) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            for (int p = 0; p < k; p++) {
                gamma(i,j) += alpha(i,p) * beta(p,j);
            }
        }
    }
}

// 2. IPJ
void gemm_IPJ(int m, int n, int k,
              double *A, int ldA,
              double *B, int ldB,
              double *C, int ldC) {
    for (int i = 0; i < m; i++) {
        for (int p = 0; p < k; p++) {
            for (int j = 0; j < n; j++) {
                gamma(i,j) += alpha(i,p) * beta(p,j);
            }
        }
    }
}

// 3. JIP
void gemm_JIP(int m, int n, int k,
              double *A, int ldA,
              double *B, int ldB,
              double *C, int ldC) {
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < m; i++) {
            for (int p = 0; p < k; p++) {
                gamma(i,j) += alpha(i,p) * beta(p,j);
            }
        }
    }
}

// 4. JPI  (geralmente uma das melhores - via AXPY / colunas)
void gemm_JPI(int m, int n, int k,
              double *A, int ldA,
              double *B, int ldB,
              double *C, int ldC) {
    for (int j = 0; j < n; j++) {
        for (int p = 0; p < k; p++) {
            for (int i = 0; i < m; i++) {
                gamma(i,j) += alpha(i,p) * beta(p,j);
            }
        }
    }
}

// 5. PIJ
void gemm_PIJ(int m, int n, int k,
              double *A, int ldA,
              double *B, int ldB,
              double *C, int ldC) {
    for (int p = 0; p < k; p++) {
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                gamma(i,j) += alpha(i,p) * beta(p,j);
            }
        }
    }
}

// 6. PJI  (geralmente uma das melhores)
void gemm_PJI(int m, int n, int k,
              double *A, int ldA,
              double *B, int ldB,
              double *C, int ldC) {
    for (int p = 0; p < k; p++) {
        for (int j = 0; j < n; j++) {
            for (int i = 0; i < m; i++) {
                gamma(i,j) += alpha(i,p) * beta(p,j);
            }
        }
    }
}

// ============================================================
// Função auxiliar para rodar um teste
// ============================================================
typedef void (*gemm_func_t)(int, int, int, double*, int, double*, int, double*, int);

double run_gemm(gemm_func_t func, int m, int n, int k,
                double *A, int ldA,
                double *B, int ldB,
                double *C, int ldC,
                int repetitions) {
    
    double total_time = 0.0;
    
    for (int rep = 0; rep < repetitions; rep++) {
        zero_matrix(C, m, n, ldC);
        
        double t0 = get_time();
        func(m, n, k, A, ldA, B, ldB, C, ldC);
        double t1 = get_time();
        
        total_time += (t1 - t0);
    }
    
    return total_time / repetitions;
}

// ============================================================
// MAIN
// ============================================================
int main(int argc, char *argv[]) {
    srand(42); // seed fixa para reprodutibilidade
    
    // Tamanhos de matriz para testar (quadradas)
    int sizes[] = {100, 200, 300, 400, 500, 600, 800, 1000, 1200};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    int repetitions = 3;   // quantas vezes roda cada teste (para média)
    
    // Nomes das ordenações
    const char *names[6] = {"IJP", "IPJ", "JIP", "JPI", "PIJ", "PJI"};
    gemm_func_t funcs[6] = {gemm_IJP, gemm_IPJ, gemm_JIP, gemm_JPI, gemm_PIJ, gemm_PJI};
    
    // Abre arquivo CSV
    FILE *csv = fopen("gemm_results.csv", "w");
    if (!csv) {
        perror("Erro ao criar CSV");
        return 1;
    }
    
    fprintf(csv, "size,ordering,time_sec,gflops\n");
    
    printf("=== Comparação das 6 Ordenações de Loops para GEMM ===\n");
    printf("Tamanho | Ordenação | Tempo (s)   | GFLOPS\n");
    printf("----------------------------------------------------\n");
    
    for (int s = 0; s < num_sizes; s++) {
        int m = sizes[s];
        int n = m;
        int k = m;
        
        int ldA = m;   // leading dimension = m (coluna-major)
        int ldB = k;
        int ldC = m;
        
        // Aloca matrizes
        double *A = (double*)malloc(ldA * k * sizeof(double));
        double *B = (double*)malloc(ldB * n * sizeof(double));
        double *C = (double*)malloc(ldC * n * sizeof(double));
        
        if (!A || !B || !C) {
            fprintf(stderr, "Erro de alocação de memória\n");
            return 1;
        }
        
        init_matrix(A, m, k, ldA);
        init_matrix(B, k, n, ldB);
        
        for (int ord = 0; ord < 6; ord++) {
            double avg_time = run_gemm(funcs[ord], m, n, k, A, ldA, B, ldB, C, ldC, repetitions);
            
            // GFLOPS = 2 * m * n * k / tempo / 1e9
            double gflops = (2.0 * m * n * k) / (avg_time * 1e9);
            
            printf("%7d | %-9s | %10.6f | %8.2f\n", m, names[ord], avg_time, gflops);
            fprintf(csv, "%d,%s,%.6f,%.2f\n", m, names[ord], avg_time, gflops);
        }
        
        printf("----------------------------------------------------\n");
        
        free(A);
        free(B);
        free(C);
    }
    
    fclose(csv);
    printf("\nResultados salvos em: gemm_results.csv\n");
    printf("Execute o script Python para gerar os gráficos.\n");
    
    return 0;
}