# Comparison

On my VM:
- Avl Tree takes ~3025ms
- Red Black Tree takes ~3670ms
- Skip List takes ~7050ms
- std::set takes ~2500ms

So std::set is the fastest, with Avl tree and Red Black tree being comparable in speed. As expected, Skip List is slower.

Of course (other than std::set), these data structures are not very optimised - my implementation of Red Black tree takes ~1000ms longer than the implementation used in std::set.

These comparisons were done using the code in comparison/ directory.
