# Manual de ejecución – Algoritmo Genético para PFSP

## 1. Requisitos

Para ejecutar el proyecto se necesita:

- **Code::Blocks** con compilador GNU GCC (MinGW) compatible con C++17.
- **Python 3.13** para generar las gráficas.
- Bibliotecas de Python: `matplotlib` y `pandas`.

## 2. Compilación y ejecución del proyecto

1. Abrir Code::Blocks y seleccionar **File → Open**.
2. Abrir el archivo `GA_PFSP.cbp`.
3. Verificar en **Project → Build options → Search directories → Compiler** que esté incluida la carpeta `include`.
![alt text](docs\SearchDirectories.png)
4. Verificar que esté habilitado el estándar C++17.
![alt text](docs\C++17.png)
5. Compilar mediante **Build → Build (Ctrl + F9)**.

Para ejecutar el algoritmo desde Code::Blocks:

1. Ingresar a **Project → Set programs' arguments**.
![alt text](docs\SetPrograms.png)
2. Seleccionar **Debug**.
3. Introducir los siguientes argumentos:

**Instancia 1**
```
"instancias\instancia1_bas1.txt" --corridas 10 --poblacion 150 --generaciones 800 --salida "resultados\Final_P150_G800"
```

**Instancia 2**
```
"instancias\instancia2_car5.txt" --corridas 10 --poblacion 150 --generaciones 800 --salida "resultados\Final_P150_G800"
```

**Instancia 3**
```
"instancias\instancia3_car3.txt" --corridas 10 --poblacion 150 --generaciones 800 --salida "resultados\Final_P150_G800"
```

![alt text](docs\Debug.png)

4. Presionar **OK** y ejecutar con **F9**.

Cada ejecución realiza 10 corridas independientes y genera los archivos `corridas.csv`, `resumen.csv`, `convergencia.csv` y `gantt.csv`.

**Nota:** Si se desea realizar una demostración sin sobrescribir los resultados finales, reemplazar la carpeta indicada en `--salida` por `resultados\Demo`.

## 3. Instalación de bibliotecas para generar las figuras

Abrir la terminal de PowerShell o VS Code en la carpeta principal `GA_PFSP`.

Ejecutar el siguiente comando para instalar las bibliotecas necesarias:

```
py -3.13 -m pip install matplotlib pandas
```

La instalación solo es necesaria la primera vez.

## 4. Generación de gráficas

Verificar que el archivo `graficar.py` se encuentre dentro de la carpeta `scripts`.

Desde la carpeta principal del proyecto, ejecutar:

```
py -3.13 scripts\graficar.py resultados\Final_P150_G800
```

Este comando procesa los archivos CSV de las tres instancias y genera automáticamente:

- `convergencia.png`: muestra la evolución del mejor makespan por generación.
- `gantt.png`: representa el orden y los tiempos de procesamiento de los trabajos en cada máquina.

## 5. Ubicación de los resultados

Las gráficas y archivos CSV quedan almacenados en:

```
GA_PFSP/
└── resultados/
    └── Final_P150_G800/
        ├── instancia1_bas1/
        ├── instancia2_car5/
        └── instancia3_car3/
```

Cada carpeta de instancia contiene sus cuatro archivos CSV y, después de ejecutar `graficar.py`, las dos imágenes PNG correspondientes.