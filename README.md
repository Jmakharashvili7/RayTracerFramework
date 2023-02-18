Low level games programming report

Jaba Makharashvili

Memory Management

The design idea behind my memory management was to develop a header and
cpp file that could be used in any project to track memory usage and be
able to find any errors in assigning memory. First major change this
file brings is overloading new and delete. When new is called it
allocates memory for the object and memory for a header and a footer.
The header is used to track the type of tracker the object needs, the
size of the actual object, a checker value which checks if the memory
was accidentally overwritten, and a pointer to the next and previous
header to create a doubly linked list. The two classes in the file are a
tracker class and a tracker manager class. A tracker class contains an
enum that makes it possible to have multiple types of trackers. If the
user decides to add another type of tracker, they simply need to add
another enum and the class supports it. This was possible due to the
custom trackers being stored within a vector in tracker manager class.
The class has three public functions which use the trackers. Add bytes,
remove bytes, and delete trackers. Add and remove functions calls a
private function called get tracker which checks the vector for the enum
type of tracker and if it does not exist it will create a new one. This
ensures that there will only be one kind of tracker for each enum. The
class itself is static and every function and the vector are all static
which makes access to the class easy from anywhere. The default tracker
is also kept separately as a static member variable. The reason for this
is to avoid recursion within overridden new. When a new tracker is made
it needs to be pushed back within the vector to be used, but vector
push_back function also calls new and due to this there will never be a
usable tracker. To overcome this problem the default tracker is kept
separate, and this keeps the code safe from infinite loops. There is
also an extra enum for trackers which ensures that there is no tracker
made for trackers and instead we add size of the vector plus the size of
default tracker to the total allocated bytes when printing it out. For
error checking the user can call WalkTheHeap function. The class has a
reference to the first header which using a doubly linked list has a
reference to the next and previous header. This way the function can go
through each allocated memory and check for errors. Currently the
function prints out the memory and then checks if the header and footer
were overwritten and if they were it prints out an error message. To
ensure all this code is only ran during debug all functions that are
called outside of the class or are public have #ifdef \_DEBUG which
ensures the code only gets run in debug mode.

![Text Description automatically
generated](./images/media/image1.png)

![A screenshot of a computer screen Description automatically generated
with medium confidence](./images/media/image2.png)

Next, I set up a memory pool which allocates memory on demand and
instead of deallocating memory it simply sets the memory as free. First
on construction the pool mallocs 1024 bytes and then each time memory is
allocated it checks how many chunks it needs and then stores the memory
in the chunks. If the memory pool does not have enough memory to
allocate new chunks it will malloc a minimum size which is currently set
to chunk size x 2(one chunk is 128 bytes). When looking for a chunk the
memory pool will go through all the chunks which are connected using a
doubly linked list and once it finds a suitable memory chunk it will
return the chunk but if the size of the memory is bigger than the free
memory variable the memory pool will allocate new memory. Once the chunk
is freed the memory pool updates the chunk to be set to free and all the
variables are updated accordingly. This makes the speed of deleting and
assigning memory much faster as we don't need to allocate and delete new
memory each time, we can simply return the memory back to the pool when
we no longer need it, and the big chunk of the memory is assigned at run
time.

![Text Description automatically
generated](./images/media/image3.png)

To find what parts of the code need to be optimized I had to run the
built-in performance profiler that is built into visual studio. After
running the performance profiler, the most notable functions were trace
which was taking 25% of the CPU, the for loop to write to the file which
took 22% of the CPU and writing to the file took 46% of the CPU. As I
was using smooth scaling my strategy was to multithread the render
function as it was getting called 120 times per frame and optimizing
only one part of the render function would seem less effective. When I
ran the application, it took very long to print out each sphere so in
theory it made sense to multithread there. Below are the results of
multithreading the render function and then the smooth scaling.

![A screenshot of a computer Description automatically generated with
medium confidence](./images/media/image4.png)

![A screenshot of a computer Description automatically generated with
medium confidence](./images/media/image5.png)

![Text Description automatically
generated](./images/media/image6.png)

As can be seen in the last picture the improved ofs.write() function
only takes 0.2% of the time which is around ten times better than the
previous implementation.

Ray Tracing Multi-Threading Optimization results. My computer is running
12 threads and 6 cores. Each test was running 5 times and the average
was input into the table.

![Chart Description automatically
generated](./images/media/image7.png)

Precise results from the testing:

![A screenshot of a computer screen Description automatically generated
with medium
confidence](./images/media/image8.png)

Using more than 100 threads seemed to have completely crashed the
application. I've tested 200 and 1000 threads and the application would
keep printing that its rendering 0 frame and would compile after 8
seconds but with no results. As can be seen from the graph
multi-threading the render function was the least effective and would
barely improve the time compared to the other two methods. The best time
achieved was with 12 threads and the time ran was 6.789 but the 6 thread
time was only milliseconds slower. The most second-best performance was
for multithreading both render and smooth scaling with threads equally
divided between them. 2 threads did not have an influence on runtime as
it was still 1 thread for each application. At 6 threads the improvement
was huge as the time was dropped all the way to 3.476 seconds and the
best time was with 12 threads at 3.322 after 12 threads the time was
falling off and the time at 100 threads was by far the worst at 14.43
seconds. The best performing function to multi-thread was undoubtedly
Smooth Scaling function. The time performance was consistently improving
until 12 threads and starting from 12 threads the time was very slightly
falling off but even at 100 threads it outperformed the other two at
their best. For smooth scaling the best time was at 12 threads at a
stunning 2.222 seconds which was 334% faster than the initial compile
time. The common pattern in all three was the fact that the best time
was always at 12 threads the reason being my computer having exactly 12
threads. Due to this if more threads were spawned the time it took to
setup the threads and the fact that they had to share the cores made the
time worse. Using less than 12 threads would be inefficient as we would
not be utilizing the full potential of the CPU.

The code provided was cache friendly since the arrays are accessed in
the correct order. For example, when going through the pixels the for
loop goes through the row left to right then moves to the next row so
the CPU can easily predict the rest of the values we access, and cache
misses are minimized. To test the performance, I rewrote the for loop to
access the array starting from the last element, but the performance
difference was quite small, after doing research I found out the cpu can
predict if we are reading the array from the end and despite taking
slightly longer can still start correctly preloading the values. After
this I tried randomly accessing the pixels and this is where the
performance difference was slightly noticeable. The compile time went
form an average of 0.140 seconds to around 0.155 seconds. Even though
the difference is small it can make a big difference in a real
application. The reason for this is due to random access to pixels the
CPU has absolutely no way to predict which index we will be accessing
next and thus almost every access will be a cache miss.

After everything was done, I noticed the time it took to write to the
file was heavily slowing down the application and decided to optimize it
to be faster. To achieve this, I created a char array which is 3 times
the size of the screen to accommodate for the rgb values. Then instead
of writing to a stream buffer I use write function of the file which
takes in size of the image and the char array to write. In result this
ended up speeding up the application down to 1.3 seconds.

Final optimization left to make was to change vector of spheres to an
array. I made a pointer of type sphere and created a #define called
SPHERE_COUNT. After refactoring code to work with the array the
application time went down to 0.9 seconds.

Once optimization was done, I downloaded a header for importing json
files by Nlohmann. I setup 4 json files that have the same properties as
the ones provided by the framework. To make this work and to incorporate
memory pool I had to alter the framework and how rendering functions
work. To test the memory pool, I decided to create a vector of sphere
pointers in the main function which could have any number of spheres
loaded and the smooth scaling function would still work as for the
simple render function I removed it and instead for testing I call
render and pass in the spheres since the whole point of simple render
was sphere setup. After setting this up I noticed the application took
roughly 2 seconds longer to compile and went from 7.4 seconds to 9.1. I
did not expect this result as I no longer need to delete the vector
every for loop nor assign anything. Only thing done in the for loop is
the render function is called and the radius and radius squared of the
sphere we're scaling is updated. The memory pool works and despite the
difference being some milliseconds it is still faster using memory pool
than using new and delete. If I am to multithread the SmoothScaling
function this would not work as multiple threads would change the same
sphere, thus in case of multithreaded smooth scaling I copy the vector
into a new vector and change the values there. The render function takes
a pointer array and if the application provides a normal array the
overload is called and it creates an array of pointer spheres and copies
the results into them making sure the original sphere array is not
affected.

![A screenshot of a computer Description automatically generated with
medium confidence](./images/media/image9.png)

![A screenshot of a computer Description automatically generated with
medium
confidence](./images/media/image10.png)

When it was time to port the application, I decided to develop a thread
pool class which checks if win32 is defined. If win 32 is defined the
application uses threads to run concurrently but if win32 is not defined
it uses vfork instead of threads. In case of forks the class keeps track
of std::threads and uses join() function to wait for all threads but if
the application uses forks it has a vector of ids and for each id it
will check if(wait(status)) and while its -1 it wont exit. Once all
forks are done the application will continue. The class has two
functions called CreateJob and WaitAllJobs which run different code
depending on which platform it runs. In case of windows, it creates a
thread and adds it to the vector but in case of linux it creates a fork
and if it's a child fork it will store the id in the vector and it will
run the job and once the job is done it calls \_exit function. This way
the code runs on both windows and linux while running same functions for
concurrency. To run linux I downloaded VirtualBox and downloaded Ubuntu
to run linux as it is very user friendly. Then I setup clion to run the
code and now everything is running fine.

Code for the linux and windows threading/forking.

![A screenshot of a computer Description automatically generated with
medium
confidence](./images/media/image11.png)![Text Description automatically
generated](./images/media/image12.png)

To make sure the application has good OOP design every class has its own
header and cpp file so the code is clean and easy to read. Main cpp file
only contains the main function and two header files to keep it short
and clean.

![Text Description automatically
generated](./images/media/image13.png)

![Text Description automatically
generated](./images/media/image14.png)
