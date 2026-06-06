# 6CS005 High Performance Computing Assignment

# 6CS005 Portfolio – High Performance Computing

## 📌 Overview

This portfolio demonstrates parallel computing techniques using CPU and GPU programming. It includes four tasks implemented using Pthreads, OpenMP, and CUDA.

Each task processes data efficiently by dividing the workload across multiple threads or GPU cores.

---

## 📁 Project Structure

```
6CS005_Portfolio/
├── README.md
├── Task1_Pthreads_WordCount/
│   ├── data/
│   │   └── input.txt
│   ├── output/
│   │   └── result.txt
│   └── src/
│       ├── word_count.c
│       └── test.c
├── Task2_OpenMP_Matrix/
│   ├── data/
│   │   └── matrices.txt
│   ├── output/
│   │   └── results.txt
│   └── src/
│       └── matrix_ops.c
├── Task3_CUDA_PasswordCrack/
│   ├── data/
│   │   └── passwords.txt
│   ├── output/
│   │   └── decrypted.txt
│   └── src/
│       └── password_crack.cu
└── Task4_CUDA_Sobel/
    ├── images/
    │   ├── input/
    │   │   └── input.png
    │   └── output/
    │       └── sobel_output.png
    ├── include/
    │   ├── stb_image.h
    │   └── stb_image_write.h
    └── src/
        └── sobel.cu
```

---

## 🔹 Task 1 – Word Count (Pthreads)

### Description

Counts the frequency of words in a text file using multithreading.

### Key Features

* File input using command line
* Multiple threads using Pthreads
* Dynamic workload distribution
* Synchronisation using mutex

### Run

```
gcc src/word_count.c -o task1 -pthread
./task1 data/input.txt 4
```

### Output

```
output/result.txt
```

---

## 🔹 Task 2 – Matrix Operations (OpenMP)

### Description

Performs multiple matrix operations in parallel.

### Operations

* Addition (A+B)
* Subtraction (A-B)
* Element-wise multiplication (A.*B)
* Element-wise division (A./B)
* Transpose (Aᵀ, Bᵀ)
* Matrix multiplication (A×B)

### Key Features

* Dynamic memory allocation (malloc)
* OpenMP parallel processing
* Dimension validation and error handling

### Run

```
gcc src/matrix_ops.c -o task2 -fopenmp -lm
./task2 data/matrices.txt 4
```

### Output

```
output/results.txt
```

---

## 🔹 Task 3 – Password Cracking (CUDA)

### Description

Uses GPU parallelism to decrypt passwords from a file.

### Key Features

* CUDA kernel implementation
* Dynamic thread/block configuration
* GPU memory management (cudaMalloc, cudaMemcpy, cudaFree)
* File input using command line

### Run

```
nvcc src/password_crack.cu -o task3
./task3 data/passwords.txt
```

### Output

```
output/decrypted.txt
```

---

## 🔹 Task 4 – Sobel Edge Detection (CUDA)

### Description

Applies Sobel edge detection on a PNG image using CUDA.

### Process

* Load image using stb_image
* Apply Sobel convolution (Gx and Gy)
* Compute edge magnitude
* Save output image

### Input

```
images/input/input.png
```

### Run

```
nvcc src/sobel.cu -o task4
./task4

```

### Output

```
images/output/sobel_output.png
```

---

## 🧠 Technologies Used

* C Programming
* Pthreads
* OpenMP
* CUDA
* File Handling
* Image Processing

---

## ⚠️ Notes

* Ensure all input files exist before running programs.
* Use correct file paths when executing commands.
* Free allocated memory to avoid leaks.

---

## ✅ Conclusion

This portfolio successfully demonstrates parallel computing using CPU and GPU techniques across multiple real-world scenarios.

---

## 👤 Author

Sithira Roneth
Undergraduate Student – BSc (Hons) Computer Science
University Of Wolverhampton
