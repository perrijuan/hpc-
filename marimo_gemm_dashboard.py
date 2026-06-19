import marimo

__generated_with = "0.9.0"
app = marimo.App(width="medium")


@app.cell
def __():
    import marimo as mo
    import pandas as pd
    import matplotlib.pyplot as plt
    import seaborn as sns
    import numpy as np
    import os
    
    sns.set_theme(style="whitegrid", font_scale=1.1)
    plt.rcParams["figure.dpi"] = 120
    return mo, pd, plt, sns, np, os


@app.cell
def __(mo):
    mo.md(
        """
        # 📊 Dashboard de Desempenho - GEMM (6 Ordenações de Loops)
        
        Este notebook visualiza os resultados do benchmark das **6 ordenações de laços** 
        para a operação `C := AB + C` (baseado no material *Loops and More Loops*).
        
        ### Ordenações testadas:
        - **IJP**, **IPJ**, **JIP**, **JPI**, **PIJ**, **PJI**
        """
    )
    return


@app.cell
def __(mo, os, pd):
    # Tenta carregar automaticamente o CSV local (adaptado para o seu gemm_results.csv)
    csv_path = "gemm_results.csv"
    
    _file = None
    if os.path.exists(csv_path):
        df = pd.read_csv(csv_path)
        mo.md(f"✅ **CSV carregado automaticamente:** `{csv_path}` ({len(df)} linhas)")
    else:
        # Fallback: upload manual se o arquivo não existir
        _file = mo.ui.file(
            filetypes=[".csv"],
            label="📁 Faça upload do arquivo gemm_results.csv (ou coloque o arquivo na mesma pasta)"
        )
        if _file.name() is not None:
            df = pd.read_csv(_file.path())
            mo.md(f"**Arquivo carregado via upload!** {len(df)} linhas")
        else:
            mo.stop(True, mo.md("⚠️ Coloque o arquivo `gemm_results.csv` na mesma pasta ou faça upload"))
    
    # Single return at the very end of cell (marimo requirement)
    if _file is not None:
        _outputs = (df, _file)
    else:
        _outputs = (df,)
    return _outputs


@app.cell
def __(df, mo):
    mo.md("### 📈 Visão Geral dos Dados")
    return (
        mo.ui.table(
            df.groupby("ordering")["gflops"]
            .agg(["mean", "max", "min", "std"])
            .round(2)
            .sort_values("mean", ascending=False)
        ),
    )


@app.cell
def __(df, mo, plt, sns):
    mo.md("### 🔥 Desempenho (GFLOPS) por Tamanho de Matriz")
    
    fig, ax = plt.subplots(figsize=(11, 6))
    
    palette = {
        "IJP": "#1f77b4", "IPJ": "#ff7f0e", "JIP": "#2ca02c",
        "JPI": "#d62728", "PIJ": "#9467bd", "PJI": "#8c564b"
    }
    
    for _ordering in ["IJP", "IPJ", "JIP", "JPI", "PIJ", "PJI"]:
        _data = df[df["ordering"] == _ordering]
        ax.plot(_data["size"], _data["gflops"], 
                marker="o", linewidth=2.5, markersize=7,
                label=_ordering, color=palette[_ordering])
    
    ax.set_xlabel("Tamanho da matriz (m = n = k)")
    ax.set_ylabel("GFLOPS")
    ax.set_title("Comparação das 6 Ordenações de Loops para GEMM")
    ax.legend(title="Ordenação", bbox_to_anchor=(1.02, 1), loc="upper left")
    ax.grid(True, alpha=0.3)
    
    mo.mpl.interactive(fig)
    return


@app.cell
def __(df, mo, plt):
    mo.md("### 📈 Speedup Relativo ao IJP (a ordenação 'natural')")
    
    baseline = df[df["ordering"] == "IJP"][["size", "gflops"]].rename(
        columns={"gflops": "baseline"}
    )
    df_speedup = df.merge(baseline, on="size")
    df_speedup["speedup"] = df_speedup["gflops"] / df_speedup["baseline"]
    
    fig2, ax2 = plt.subplots(figsize=(11, 6))
    
    for _ordering in ["IPJ", "JIP", "JPI", "PIJ", "PJI"]:
        _data = df_speedup[df_speedup["ordering"] == _ordering]
        ax2.plot(_data["size"], _data["speedup"], 
                 marker="o", linewidth=2.5, markersize=7, label=_ordering)
    
    ax2.axhline(y=1.0, color="gray", linestyle="--", linewidth=2, label="IJP (baseline = 1.0)")
    ax2.set_xlabel("Tamanho da matriz (m = n = k)")
    ax2.set_ylabel("Speedup vs IJP")
    ax2.set_title("Quanto mais rápido cada ordenação é em relação ao IJP")
    ax2.legend(title="Ordenação")
    ax2.grid(True, alpha=0.3)
    
    mo.mpl.interactive(fig2)
    return


@app.cell
def __(df, mo, plt, sns):
    mo.md("### 📦 Distribuição de Desempenho por Ordenação")
    
    fig3, ax3 = plt.subplots(figsize=(10, 6))
    
    order = ["IJP", "IPJ", "JIP", "JPI", "PIJ", "PJI"]
    palette_box = ["#1f77b4", "#ff7f0e", "#2ca02c", "#d62728", "#9467bd", "#8c564b"]
    
    sns.boxplot(data=df, x="ordering", y="gflops", order=order, 
                hue="ordering", palette=palette_box, ax=ax3, legend=False)
    sns.swarmplot(data=df, x="ordering", y="gflops", order=order,
                  color="black", alpha=0.5, size=4, ax=ax3)
    
    ax3.set_xlabel("Ordenação dos Laços")
    ax3.set_ylabel("GFLOPS")
    ax3.set_title("Variação de Desempenho por Tipo de Ordenação")
    
    mo.mpl.interactive(fig3)
    return


@app.cell
def __(mo):
    mo.md(
        """
        ---
        ### 💡 Observações Importantes (do material)
        
        - As ordenações **JPI** e **PJI** costumam ser as mais rápidas.
        - Isso acontece porque o laço interno acessa a memória de forma **contígua** 
          (boa localidade espacial no column-major storage).
        - A ordenação **IJP** (mais "natural" de se escrever) geralmente é uma das piores.
        """
    )
    return


if __name__ == "__main__":
    app.run()