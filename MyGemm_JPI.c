void Gemm_JPI(int m, int n, int k,
              double *A, int ldA,
              double *B, int ldB,
              double *C, int ldC)
{
    for (int j = 0; j < n; j++) {
        for (int p = 0; p < k; p++) {
            for (int i = 0; i < m; i++) {
                C[j*ldC + i] += A[p*ldA + i] * B[j*ldB + p];
            }
        }
    }
}

// =============================================
// MyGemm_JPI - Blocked version chamando JPI
// =============================================
void MyGemm_JPI(int m, int n, int k,
                double *A, int ldA,
                double *B, int ldB,
                double *C, int ldC,
                int MB, int NB, int KB)
{
    for (int j = 0; j < n; j += NB) {
        int jb = (j + NB > n) ? n - j : NB;

        for (int i = 0; i < m; i += MB) {
            int ib = (i + MB > m) ? m - i : MB;

            for (int p = 0; p < k; p += KB) {
                int pb = (p + KB > k) ? k - p : KB;

                Gemm_JPI(ib, jb, pb,
                         &alpha(i, p), ldA,
                         &beta(p, j), ldB,
                         &gamma(i, j), ldC);
            }
        }
    }
}