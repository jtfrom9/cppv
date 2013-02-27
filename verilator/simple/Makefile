TARGET=obj_dir/Vtest

all: $(TARGET)

$(TARGET):
	verilator -Wall --cc test.v --exe test.cpp
	cd obj_dir;	make VM_USER_CFLAGS="-g" -j -f Vtest.mk Vtest

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf obj_dir *~  $(TARGET)
