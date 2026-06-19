// Gemm_PJI - Versão simples (não bloqueada)
// Laços: p -> j -> i
void Gemm_PJI(int m, int n, int k,
              double *A, int ldA,
              double *B, int ldB,
              double *C, int ldC)
{
    for (int p = 0; p < k; p++) {
        for (int j = 0; j < n; j++) {
            for (int i = 0; i < m; i++) {
                C[j*ldC + i] += A[p*ldA + i] * B[j*ldB + p];
            }
        }
    }
}