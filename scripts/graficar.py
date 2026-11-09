"""
Genera las figuras del informe a partir de los CSV que produce ga_pfsp:
  - curva de convergencia (mejor corrida y promedio de las corridas)
  - diagrama de Gantt de la mejor solucion

Uso:  python scripts/graficar.py [carpeta_resultados]
Requiere: pip install pandas matplotlib
"""
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd


def graficar_convergencia(carpeta: Path) -> None:
    conv = pd.read_csv(carpeta / "convergencia.csv")
    corridas = pd.read_csv(carpeta / "corridas.csv")
    mejor = int(corridas.loc[corridas["mejor_makespan"].idxmin(), "corrida"])

    fig, ax = plt.subplots(figsize=(8, 4.5))
    ax.step(conv["generacion"], conv[f"corrida{mejor}"], where="post",
            label=f"Mejor corrida ({mejor})", linewidth=2)
    ax.plot(conv["generacion"], conv["promedio"], linestyle="--",
            label="Promedio de las corridas")
    ax.set_xlabel("Generacion")
    ax.set_ylabel("Mejor makespan acumulado")
    ax.set_title(f"Curva de convergencia - {carpeta.name}")
    ax.grid(alpha=0.3)
    ax.legend()
    fig.tight_layout()
    fig.savefig(carpeta / "convergencia.png", dpi=200)
    plt.close(fig)


def graficar_gantt(carpeta: Path) -> None:
    gantt = pd.read_csv(carpeta / "gantt.csv")
    maquinas = sorted(gantt["maquina"].unique(), key=lambda s: int(s[1:]))
    trabajos = list(dict.fromkeys(gantt["trabajo"]))  # en orden de la secuencia
    colores = plt.cm.tab20.colors
    color_de = {t: colores[i % len(colores)] for i, t in enumerate(trabajos)}
    cmax = gantt["fin"].max()

    fig, ax = plt.subplots(figsize=(10, 1 + 0.7 * len(maquinas)))
    for _, fila in gantt.iterrows():
        y = maquinas.index(fila["maquina"])
        ancho = fila["fin"] - fila["inicio"]
        ax.barh(y, ancho, left=fila["inicio"], color=color_de[fila["trabajo"]],
                edgecolor="black", height=0.6)
        ax.text(fila["inicio"] + ancho / 2, y, fila["trabajo"], ha="center",
                va="center", fontsize=8)
    ax.axvline(cmax, color="red", linestyle="--", label=f"Cmax = {cmax}")
    ax.set_yticks(range(len(maquinas)))
    ax.set_yticklabels(maquinas)
    ax.invert_yaxis()
    ax.set_xlabel("Tiempo")
    ax.set_title(f"Diagrama de Gantt - {carpeta.name} - [{', '.join(trabajos)}]")
    ax.legend(loc="upper left", bbox_to_anchor=(1.0, 1.0))
    fig.tight_layout()
    fig.savefig(carpeta / "gantt.png", dpi=200)
    plt.close(fig)


def main() -> None:
    raiz = Path(sys.argv[1] if len(sys.argv) > 1 else "resultados")
    carpetas = [c for c in sorted(raiz.iterdir()) if (c / "convergencia.csv").exists()]
    if not carpetas:
        print(f"No se encontraron resultados en {raiz}/")
        return
    for carpeta in carpetas:
        graficar_convergencia(carpeta)
        graficar_gantt(carpeta)
        print(f"Figuras generadas en {carpeta}/ (convergencia.png, gantt.png)")


if __name__ == "__main__":
    main()
