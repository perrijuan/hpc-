"""
plot_gemm_results.py
Script para visualizar os resultados do benchmark GEMM (6 ordenações de loops)

Uso:
    python plot_gemm_results.py

Gera:
    - gemm_performance.png (gráfico principal)
    - gemm_speedup.png (speedup relativo ao IJP)
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# Configuração visual
sns.set_theme(style="whitegrid", font_scale=1.2)
plt.rcParams['figure.figsize'] = (12, 7)
plt.rcParams['savefig.dpi'] = 150

# Lê os resultados
df = pd.read_csv("gemm_results.csv")

print("Primeiras linhas do CSV:")
print(df.head(12))
print("\nEstatísticas por ordenação:")
print(df.groupby("ordering")["gflops"].describe())

# ============================================================
# GRÁFICO 1: GFLOPS vs Tamanho da Matriz (todas as ordenações)
# ============================================================
plt.figure(figsize=(13, 8))

# Cores bonitas
palette = {
    "IJP": "#1f77b4",   # azul
    "IPJ": "#ff7f0e",   # laranja
    "JIP": "#2ca02c",   # verde
    "JPI": "#d62728",   # vermelho
    "PIJ": "#9467bd",   # roxo
    "PJI": "#8c564b",   # marrom
}

for ordering in df["ordering"].unique():
    data = df[df["ordering"] == ordering]
    plt.plot(data["size"], data["gflops"], 
             marker='o', linewidth=2.5, markersize=8,
             label=ordering, color=palette.get(ordering, "gray"))

plt.xlabel("Tamanho da matriz (m = n = k)", fontsize=14)
plt.ylabel("Desempenho (GFLOPS)", fontsize=14)
plt.title("Comparação das 6 Ordenações de Loops para GEMM\n(C := AB + C) — Column-major", fontsize=16, pad=20)
plt.legend(title="Ordenação", fontsize=12, title_fontsize=13)
plt.grid(True, alpha=0.3)
plt.tight_layout()

plt.savefig("gemm_performance.png", dpi=150, bbox_inches="tight")
print("\nGráfico salvo: gemm_performance.png")

# ============================================================
# GRÁFICO 2: Speedup relativo ao IJP (a ordenação "natural")
# ============================================================
plt.figure(figsize=(13, 8))

# Pega o IJP como baseline
baseline = df[df["ordering"] == "IJP"][["size", "gflops"]].rename(columns={"gflops": "baseline_gflops"})
df_merged = df.merge(baseline, on="size")
df_merged["speedup"] = df_merged["gflops"] / df_merged["baseline_gflops"]

for ordering in df_merged["ordering"].unique():
    if ordering == "IJP":
        continue
    data = df_merged[df_merged["ordering"] == ordering]
    plt.plot(data["size"], data["speedup"], 
             marker='o', linewidth=2.5, markersize=8,
             label=ordering, color=palette.get(ordering, "gray"))

plt.axhline(y=1.0, color="gray", linestyle="--", linewidth=1.5, label="IJP (baseline)")
plt.xlabel("Tamanho da matriz (m = n = k)", fontsize=14)
plt.ylabel("Speedup relativo ao IJP", fontsize=14)
plt.title("Speedup das Ordenações em relação ao IJP\n(Valores > 1.0 = mais rápido que IJP)", fontsize=16, pad=20)
plt.legend(title="Ordenação", fontsize=12, title_fontsize=13)
plt.grid(True, alpha=0.3)
plt.tight_layout()

plt.savefig("gemm_speedup.png", dpi=150, bbox_inches="tight")
print("Gráfico salvo: gemm_speedup.png")

# ============================================================
# GRÁFICO 3: Boxplot por ordenação (melhor visualização da variação)
# ============================================================
plt.figure(figsize=(12, 7))
sns.boxplot(data=df, x="ordering", y="gflops", palette=palette, order=["IJP","IPJ","JIP","JPI","PIJ","PJI"])
sns.swarmplot(data=df, x="ordering", y="gflops", color="black", alpha=0.6, size=4)

plt.xlabel("Ordenação dos Laços", fontsize=14)
plt.ylabel("GFLOPS", fontsize=14)
plt.title("Distribuição de Desempenho por Ordenação de Loops", fontsize=16)
plt.tight_layout()

plt.savefig("gemm_boxplot.png", dpi=150, bbox_inches="tight")
print("Gráfico salvo: gemm_boxplot.png")

print("\n=== Todos os gráficos foram gerados com sucesso! ===")