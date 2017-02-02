#include <iostream>
#include <vector>

int main(void)
{
  std::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.resize(0);
  v[1] = 42;  // Oops. Out of bounds access, but Valgrind is silent

  std::cout << "v[0]=" << v[0] << "; v[1]=" << v[1] << std::endl;
  return 0;
}


/*
http://stackoverflow.com/questions/5774794/heap-corruption-not-detected-by-valgrind-or-electric-fence-should-i-be-suspicio

Q:
I recently encountered my first battle (solved) with heap corruption. On my linux machine at home the culprit code exits without error using valgrind and electric-fence(with gdb). Yet on the windows machine in our lab, I consistently get the heap corruption related error message from VS described in my referenced post.

Is it surprising (or at least uncommon) that valgrind and electric fence wouldn't detect such a problem? Someone else mentioned a possibly similar bug that eluded valgrind in a answer here. What might be some reasons why this problem wouldn't be detected by these tools? Is there any reason to doubt that error is in fact heap corruption?

Update: As mentioned in the post describing the original problem, I found that the problem was due to having pointers to elements in a std::vector, which became bad. Replacing the vectors with std::list (to which pointers don't become invalid when adding new elements) fixed the problem. So getting back to my question about why valgrind didn't detect the problem, I ask if there are any recommendations about how to avoid a similar situation in the future, namely a memory problem that isn't detected by valgrind which is one of my favorite tools. Obviously getting a better understanding of how STL works would be a good idea. Perhaps I need to be more assertive with assert in my programming, etc.

A:
So the apparent reason that Valgrind failed to detect your heap corruption is that the corruption did not happen with GCC STL implementation at all (i.e. there was no error to detect).

Unfortunately, Valgrind operates at much lower level than STL, and so many bugs remain undetected. For example:

std::vector<int> v;
v.push_back(1);
v.push_back(2);
v.resize(0);
v[1] = 42;  // Oops. Out of bounds access, but Valgrind is silent
Fortunately, GCC STL has a special debugging mode, designed to catch many such problems. Try building your original code with -D_GLIBCXX_DEBUG. It will likely catch the original problem, and may catch more problems you don't yet know about.
*/

