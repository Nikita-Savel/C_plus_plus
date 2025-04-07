 #include <iostream>

 long long sum = 0;

 bool check(int index, int* current, int current_size) {
   for (int i = 0; i < current_size; i++) {
     if (index == current[i]) {
       return true;
     }
   }
   return false;
 }

 void sum_multiply(int* input, int size_mas, int** mas, int* current, int current_size) {
   if (current_size == size_mas) {
     int multiply = 1;
     for (int i = 0; i < size_mas; i++) {
       multiply *= mas[i][current[i]];
     }
     sum += multiply;
   } else {
     for (int i = 0; i < input[current_size]; i++) {
       if (!check(i, current, current_size)) {
         current[current_size] = i;
         sum_multiply(input, size_mas, mas, current, current_size + 1);
       }
     }
   }
 }

 int main(int argc, char* argv[]) {
   if (argc == 1) {
     std::cout << "error" << std::endl;
     return 1;
   }
   int size_mas = argc - 1;
   int* input = (int*) calloc(size_mas, sizeof(int));
   for (int i = 1; i < argc; i++) {
     input[i - 1] = atoi(argv[i]);
   }

   int** mas = (int**) calloc(size_mas, sizeof(int*));
   for (int i = 1; i < argc; i++) {
     mas[i - 1] = (int*) calloc(input[i - 1], sizeof(int));
   }

   for (int i = 1; i < argc; i++) {
     for (int j = 0; j < input[i - 1]; j++) {
       std::cin >> mas[i - 1][j];
     }
   }

   int* current = (int*) calloc(size_mas, sizeof(int));
   int current_size = 0;
   sum_multiply(input, size_mas, mas, current, current_size);
   std::cout << sum << std::endl;
   free(input);
   for (int i = 0; i < size_mas; i++) {
     free(mas[i]);
   }
   free(mas);
   free(current);
 }
