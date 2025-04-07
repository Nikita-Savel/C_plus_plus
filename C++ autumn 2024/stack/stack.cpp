#include <iostream>
#include <cstring>

const ssize_t default_capacity = 2;

void push(char**& stack, int& stack_size, int& stack_cap, const char* str) {
  while (stack_size > stack_cap - 1) {
    stack_cap *= 2;
    char** temp = (char**) calloc(stack_cap, sizeof(char*));
    for (int i = 0; i < stack_size; i++) {
      temp[i] = (char*) calloc(strlen(stack[i]) + 1, sizeof(char));
      strncpy(temp[i], stack[i], strlen(stack[i]));
      free(stack[i]);
    }
    free(stack);
    stack = temp;
  }
  stack[stack_size] = (char*) calloc(strlen(str) + 1, sizeof(char));
  strcpy(stack[stack_size], str);
  stack_size++;

  std::cout << "ok" << std::endl;
}

void pop(char**& stack, int& stack_size, int& stack_cap) {
  if (stack_size == 0) {
    std::cout << "error" << std::endl;
  } else {
    stack_size--;
    std::cout << stack[stack_size] << std::endl;
    free(stack[stack_size]);
    if (stack_cap >= 4 * stack_size && stack_cap > 1) {
      stack_cap /= 2;
      char** temp = (char**) calloc(stack_cap, sizeof(char*));
      for (int i = 0; i < stack_size; i++) {
        temp[i] = (char*) calloc(strlen(stack[i]) + 1, sizeof(char));
        strcpy(temp[i], stack[i]);
        free(stack[i]);
      }
      free(stack);
      stack = temp;
    }
  }
}

void back(char**& stack, const int stack_size) {
  if (stack_size == 0) {
    std::cout << "error" << std::endl;
  } else {
    std::cout << stack[stack_size - 1] << std::endl;
  }
}

void size(const int stack_size) {
  std::cout << stack_size << std::endl;
}

void clear(char**& stack, int& stack_size, int& stack_cap) {
  for (int i = 0; i < stack_size; i++) {
    free(stack[i]);
  }
  stack_size = 0;
  stack_cap = default_capacity;
  std::cout << "ok" << std::endl;
}

void exit(char**& stack, int& stack_size, int& stack_cap) {
  std::cout << "bye" << std::endl;
  if (stack != nullptr) {
    for (int i = 0; i < stack_size; i++) {
      free(stack[i]);
    }
    free(stack);
    stack_size = 0;
    stack_cap = default_capacity;
  }
}

void realloc_string(char*& str, const int& str_len, int& str_cap) {
  str_cap *= 2;
  char* temp = (char*) calloc(str_cap + 1, sizeof(char));
  strncpy(temp, str, str_len);
  free(str);
  str = temp;
}

void read_string(char*& str, int& str_len, int& str_cap) {
  char ch = '\0';
  while ((scanf("%c", &ch)) == 1) {
    if (ch != '\n') {
      str_len++;
      if (str_cap - 1 < str_len) {
        realloc_string(str, str_len, str_cap);
      }
      str[str_len - 1] = ch;
    } else {
      break;
    }
  }
}

int main() {
  char** stack = (char**) calloc(default_capacity, sizeof(char*));
  int stack_size = 0;
  int stack_cap = default_capacity;
  char* str = (char*) calloc(default_capacity, sizeof(char));
  int str_len = 0;
  int str_cap = default_capacity;
  while (true) {
    read_string(str, str_len, str_cap);
    if (str[0] == 'p' && str[1] == 'u') {
      const char* word = strtok(str, " ");
      word = strtok(nullptr, " ");
      char* word_copy = strdup(word);
      push(stack, stack_size, stack_cap, word_copy);
      free(word_copy);
    } else if (str[0] == 'p' && str[1] == 'o') {
      pop(stack, stack_size, stack_cap);
    } else if (str[0] == 'b') {
      back(stack, stack_size);
    } else if (str[0] == 's') {
      size(stack_size);
    } else if (str[0] == 'c') {
      clear(stack, stack_size, stack_cap);
    } else if (str[0] == 'e') {
      exit(stack, stack_size, stack_cap);
      free(str);
      break;
    }
    for (int i = 0; i < str_len; ++i) {
      str[i] = '\0';
    }
    str_len = 0;
    str_cap = default_capacity;
  }
  return 0;
}
