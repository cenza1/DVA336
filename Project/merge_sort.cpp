#include <algorithm>
#include <cassert>
#include <immintrin.h>
#include <iostream>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

using namespace std;

// The parallization with omp is only worth to do over a certain threshold,
// since the overhead cost gets larger than the time saved when comparing a small
// amount of elements.
const vector<int>::size_type GRANULARITY_THRESHOLD = 100;

int *initializeRandomizedArray(int *arr, const int size) {
    arr = new int[size];

    for (int i = 0; i < size; i++) {
        arr[i] = rand();
    }
    return arr;
}

vector<int> merge(vector<int> left, vector<int> right) {
    vector<int> sorted;

    vector<int>::size_type leftIndex = 0, rightIndex = 0;
    int valueToWrite = 0;
    while (leftIndex < left.size() && rightIndex < right.size()) {
        if (left[leftIndex] <= right[rightIndex]) {
            valueToWrite = left[leftIndex];
            leftIndex++;
        } else {
            valueToWrite = right[rightIndex];
            rightIndex++;
        }
        sorted.push_back(valueToWrite);
    }

    for (; leftIndex < left.size(); leftIndex++) {
        sorted.push_back(left[leftIndex]);
    }
    for (; rightIndex < right.size(); rightIndex++) {
        sorted.push_back(right[rightIndex]);
    }

    return sorted;
}

void mergeSort_seq(vector<int> &arr) {
    if (arr.size() <= 1) {
        return;
    }
    size_t mid = arr.size() / 2;

    vector<int> left(arr.begin(), arr.begin() + mid);
    vector<int> right(arr.begin() + mid, arr.end());

    mergeSort_seq(left);
    mergeSort_seq(right);
    arr = merge(left, right);
}

vector<int> merge_vec(vector<int> left, vector<int> right) {
    vector<int> sorted;

    vector<int>::size_type leftIndex = 0, rightIndex = 0;
    int valueToWrite = 0;
    while (leftIndex < left.size() && rightIndex < right.size()) {
        if (left[leftIndex] <= right[rightIndex]) {
            valueToWrite = left[leftIndex];
            leftIndex++;
        } else {
            valueToWrite = right[rightIndex];
            rightIndex++;
        }
        sorted.push_back(valueToWrite);
    }

    for (; leftIndex < left.size(); leftIndex++) {
        sorted.push_back(left[leftIndex]);
    }
    for (; rightIndex < right.size(); rightIndex++) {
        sorted.push_back(right[rightIndex]);
    }

    return sorted;
}

int *createSubArr(int *arr, const int start, const int subSize) {
    int *subArr = new int[subSize];
    for (int i = 0; i < subSize; i++) {
        subArr[i] = arr[start + i];
    }
    return subArr;
}

void printArray(int *arr, const int size) {
    for (int i = 0; i < size; i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}

void mergeSort_par(vector<int> &arr) {
    if (arr.size() <= 1) {
        return;
    }
    size_t mid = arr.size() / 2;

    vector<int> left(arr.begin(), arr.begin() + mid);
    vector<int> right(arr.begin() + mid, arr.end());

    // Only parallize the left and right merges if it's worth the overhead cost.
    if (arr.size() <= GRANULARITY_THRESHOLD) {
        mergeSort_par(left);
        mergeSort_par(right);
    } else {
#pragma omp task shared(left)
        mergeSort_par(left);
#pragma omp task shared(right)
        mergeSort_par(right);
#pragma omp taskwait
    }

    arr = merge_vec(left, right);
}

bool isSorted(vector<int> arr) {
    for (size_t i = 1; i < arr.size(); i++) {
        if (arr[i] < arr[i - 1]) {
            return false;
        }
    }
    return true;
}

void printSortState(vector<int> arr) {
    bool sorted = isSorted(arr);
    sorted ? printf("Array is sorted!\n") : printf("Array is not sorted!\n");
}

void copyArr(int *original, int *copyTo, const int size) {
    for (int i = 0; i < size; i++) {
        copyTo[i] = original[i];
    }
}

void compare_performance(vector<int> arr) {
    printSortState(arr);
    // printArray(arr, size);

    // Perform sequential sort
    vector<int> seqArr = arr;
    double ts = omp_get_wtime();
    mergeSort_seq(seqArr);
    ts = omp_get_wtime() - ts;
    printSortState(seqArr);
    printf("Sequential time: %f\n", ts);
    // printArray(seqArr, size);

    // Sort with parallel sort to compare
    vector<int> parArr = arr;
    double tp = omp_get_wtime();
#pragma omp parallel
    {
#pragma omp single nowait
        mergeSort_par(parArr);
    }
    tp = omp_get_wtime() - tp;
    printSortState(parArr);
    printf("Parallell time: %f\n", tp);
    // printArray(parArr, size);
}

int main(int argc, char *argv[]) {
    srand(time(nullptr));
    const int arraySize = atoi(argv[1]);
    vector<int> randomizedArr(arraySize);
    generate(randomizedArr.begin(), randomizedArr.end(), rand);

    compare_performance(randomizedArr);

    return EXIT_SUCCESS;
}