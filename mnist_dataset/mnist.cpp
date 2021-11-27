#include "mnist.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>

uint32_t reverseBits(uint32_t value)
{
  return (value << 24) | ((value << 8) & 0x00FF0000) |
         ((value >> 8) & 0X0000FF00) | (value >> 24);
}

int reverseInt(int n)
{
  const int bytes = 4;
  unsigned char ch[bytes];
  for (int i = 0; i < bytes; i++)
  {
    ch[i] = (n >> i * 8) & 255;
  }
  int p = 0;
  for (int i = 0; i < bytes; i++)
  {
    p += (int)ch[i] << (bytes - i - 1) * 8;
  }
  return p;
}

MNIST::MNIST(char *images_filename, char *labels_filename, bool shuffle)
{
  if (shuffle)
    seed = std::chrono::system_clock::now().time_since_epoch().count();

  std::cout << "Creating Dataset" << std::endl;

  label_size_ = 1;

  std::cout << "Parsing images file" << std::endl;

  parse_images_file(images_filename);

  std::cout << "Parsing labels file" << std::endl;

  parse_labels_file(labels_filename);

  std::cout << "Parsed labels file" << std::endl;

  if (shuffle)
  {
    std::shuffle(images.begin(), images.end(),
                 std::default_random_engine(seed));
    std::shuffle(labels.begin(), labels.end(),
                 std::default_random_engine(seed));
  }
}

void MNIST::shuffle()
{
  unsigned seed_temp =
      std::chrono::system_clock::now().time_since_epoch().count();
  std::shuffle(images.begin(), images.end(),
               std::default_random_engine(seed_temp));
  std::shuffle(labels.begin(), labels.end(),
               std::default_random_engine(seed_temp));
}

void MNIST::parse_images_file(char *images_file)
{
  std::ifstream fd(images_file, std::ios::in | std::ios::binary);
  char data[4];

  // read metadata
  unsigned int magic_number, rows, cols, i = 0;
  char pixel;
  fd.read((char *)&magic_number, sizeof(magic_number));
  magic_number = reverseInt(magic_number);
  fd.read((char *)&dataset_size_, sizeof(dataset_size_));
  dataset_size_ = reverseInt(dataset_size_);
  fd.read(data, sizeof(unsigned));
  fd.read((char *)&rows, sizeof(rows));
  rows = reverseInt(rows);
  fd.read((char *)&cols, sizeof(cols));
  cols = reverseInt(cols);
  input_size_ = rows * cols;
  std::cout << input_size_ << " " << rows << " " << cols << " " << dataset_size_
            << std::endl;
  images.resize(dataset_size_);
  data[0] = 0;
  data[1] = 0;
  data[2] = 0;
  while ((!fd.eof()) && (i < dataset_size_))
  {
    // std::cout << "Image Number " << i << std::endl;
    images[i].resize(input_size_);
    for (int j = 0; j < input_size_; j++)
    {
      fd.read(&data[3], 1);
      images[i][j] = reverseBits(*((unsigned int *)data)) / 255.0;
    }
    i++;
  }
  fd.close();
}

void MNIST::parse_labels_file(char *label_file)
{
  std::fstream fd;
  unsigned int magic_number, i = 0;
  char value;
  char data[4];
  fd.open(label_file, std::ios::in | std::ios::binary);
  fd.read(data, sizeof(unsigned));
  magic_number = reverseBits(*((unsigned int *)data));
  fd.read(data, sizeof(unsigned));
  dataset_size_ = reverseBits(*((unsigned int *)data));

  labels.resize(dataset_size_);
  while ((!fd.eof()) && (i < dataset_size_))
  {
    fd.read(&value, 1);
    labels[i] = int(value);
    i++;
  }
  fd.close();
}

size_t MNIST::getInputDim()
{
  return input_size_;
}

size_t MNIST::getLabelDim()
{
  return 10;
}

size_t MNIST::getDatasetSize()
{
  return dataset_size_;
}

void MNIST::get_item(int index, float *data, float *label)
{
  memcpy(data, images[index].data(), sizeof(float) * input_size_);
  memcpy(label, &labels[index], sizeof(float) * label_size_);
}

void MNIST::get_item_range(int start, int end, float *data_batch,
                           float *label_batch)
{
  for (int i = start; i < end; i++)
  {
    memcpy(data_batch + (i - start) * input_size_, images[i].data(),
           sizeof(float) * input_size_);
    memcpy(label_batch + (i - start) * label_size_, &labels[i],
           sizeof(float) * label_size_);
  }
}
