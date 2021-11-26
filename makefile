# For windows && opencv 4.2
# nvcc -w -I ..\opencv\build\include -L ..\opencv\build\x64\vc15\lib main.cu network.cu layers/layers.cu layers/pooling_layer.cu layers/relu_layer.cu layers/input_layer.cu layers/conv_layer.cu layers/fc_layer.cu layers/softmax_layer.cu layers/flatten_layer.cu kernels/softmax_kernel.cu kernels/update_kernel.cu kernels/fc_kernel.cu kernels/transpose_kernel.cu mnist_dataset/mnist.cpp data_core/data_loader.cu vmm/vmm.cpp trainer/utils.cu trainer/full_mem_trainer.cu trainer/min_mem_trainer.cu trainer/prefetching_heuristic_fetch_next.cu trainer/prefetching_heuristic_half_window.cu -lcudnn -lcublas -o test -lopencv_world420
# home = /lustre/ssingh37/MoDNN
layers = conv_layer.o fc_layer.o flatten_layer.o input_layer.o layers.o pooling_layer.o softmax_layer.o relu_layer.o
layers_headers = layers/conv_layer.h layers/fc_layer.h layers/flatten_layer.h layers/input_layer.h layers/layers.h layers/pooling_layer.h layers/softmax_layer.h layers/relu_layer.h
kernels = fc_kernel.o transpose_kernel.o softmax_kernel.o update_kernel.o
trainers = full_mem_trainer.o min_mem_trainer.o prefetching_heuristic_half_window.o prefetching_heuristic_fetch_next.o offload_when_needed.o

cc = nvcc
flags = -arch=sm_35 -std=c++11
nvidia_flags = -lcudnn -lcublas
opencv_flags = -lopencv_imgcodecs -lopencv_imgproc -lopencv_core
CFLAGS = -I$(CUDNN_INCDIR)
LDFLAGS = -L$(CUDNN_LIBDIR) 



all: main.o network.o $(layers) $(kernels) mnist.o data_loader.o vmm.o $(trainers) utils.o
	$(cc) $(CFLAGS) $(LDFLAGS) $(flags) -o test main.o network.o $(layers) $(kernels) $(trainers) mnist.o data_loader.o vmm.o utils.o $(nvidia_flags) 

main.o: main.cu layers/layers.h mnist_dataset/mnist.h data_core/data_loader.h trainer/trainer.h
	$(cc) -c $(CFLAGS) $(flags) main.cu $(nvidia_flags)

network.o:  network.cu $(layer_headers) vmm/vmm.h
	$(cc) -c $(CFLAGS) $(flags) network.cu $(nvidia_flags)

$(layers): %.o: layers/%.cu layers/%.h
	$(cc) -c $(CFLAGS) $(flags) $< -o $@ $(nvidia_flags)

$(kernels): %.o: kernels/%.cu layers/layers.h
	$(cc) -c $(CFLAGS) $(flags) $< -o $@ $(nvidia_flags)

$(trainers): %.o: trainer/%.cu trainer/trainer.h
	$(cc) -c $(CFLAGS) $(flags) $< -o $@ $(nvidia_flags)

mnist.o : mnist_dataset/mnist.cpp mnist_dataset/mnist.h data_core/dataset.h 
	$(cc) -c $(CFLAGS) $(flags) mnist_dataset/mnist.cpp $(nvidia_flags)

data_loader.o : data_core/data_loader.cu data_core/data_loader.h
	$(cc) -c $(CFLAGS) $(flags) data_core/data_loader.cu $(nvidia_flags)

vmm.o : vmm/vmm.cpp vmm/vmm.h
	$(cc) -c $(CFLAGS) $(flags) vmm/vmm.cpp $(nvidia_flags)


utils.o : trainer/utils.cu trainer/trainer.h
	$(cc) -c $(CFLAGS) $(flags) trainer/utils.cu $(nvidia_flags)

clean:
	rm $(layers) $(kernels) test mnist.o data_loader.o vmm.o

submit: all
	sbatch --share submit.sh 

run: all
	./test
