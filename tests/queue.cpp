#include "../libs/queue.h"

#include <iostream>
#include <complex>

/*
 * A small test for Queue<T> to ensure that it works
 * under different types of T
 *
 * It is not an exhaustive test.
 * */

namespace {
    const int QUEUE_MAXSIZE = 10;
}

void raise_if_false(bool ok) {
    if (!ok)
        throw std::runtime_error("assertion failed");
}

void test_non_blocking_queue__int() {
    Queue<int> q(QUEUE_MAXSIZE);
    int val;
    bool ok;

    // We expect to be able to push N elements for a N limit
    // queue without trouble
    for (int i = 0;  i < QUEUE_MAXSIZE; ++i) {
        ok = q.try_push(i);
        raise_if_false(ok);
    }

    // The N+1 element however, should fail
    ok = q.try_push(999);
    raise_if_false(!ok);

    // A pop should work just fine, retrieving
    // the first pushed element.
    ok = q.try_pop(val);
    raise_if_false(ok);
    raise_if_false(val == 0);

    // Now that we made room we can push one more element
    ok = q.try_push(999);
    raise_if_false(ok);

    // We expect to pop all the elements of the queue in
    // a FIFO order (in the loop we pop N-1 elements)
    for (int i = 1;  i < QUEUE_MAXSIZE; ++i) {
        ok = q.try_pop(val);
        raise_if_false(ok);
        raise_if_false(val == i);
    }

    // Pop the last pushed value
    ok = q.try_pop(val);
    raise_if_false(ok);
    raise_if_false(val == 999);

    // Push some values...
    q.push(42);
    q.push(57);

    // ...close the queue
    q.close();

    // and check that we cannot push anything else
    try {
        q.try_push(47);
        raise_if_false(false);
    } catch (const ClosedQueue&) {
        raise_if_false(true);
    }

    // but we can pop until the queue gets empty
    val = q.pop();
    raise_if_false(val == 42);

    val = q.pop();
    raise_if_false(val == 57);
    try {
        q.try_pop(val);
        raise_if_false(false);
    } catch (const ClosedQueue&) {
        raise_if_false(true);
    }

    std::cout << "[OK] test_non_blocking_queue__int\n";
}

void test_non_blocking_queue__complex() {
    Queue<std::complex<int>> q(QUEUE_MAXSIZE);
    std::complex<int> val;
    bool ok;

    // We expect to be able to push N elements for a N limit
    // queue without trouble
    for (int i = 0;  i < QUEUE_MAXSIZE; ++i) {
        ok = q.try_push(i);
        raise_if_false(ok);
    }

    // The N+1 element however, should fail
    ok = q.try_push(999);
    raise_if_false(!ok);

    // A pop should work just fine, retrieving
    // the first pushed element.
    ok = q.try_pop(val);
    raise_if_false(ok);
    raise_if_false(val == 0);

    // Now that we made room we can push one more element
    ok = q.try_push(999);
    raise_if_false(ok);

    // We expect to pop all the elements of the queue in
    // a FIFO order (in the loop we pop N-1 elements)
    for (int i = 1;  i < QUEUE_MAXSIZE; ++i) {
        ok = q.try_pop(val);
        raise_if_false(ok);
        raise_if_false(val == i);
    }

    // Pop the last pushed value
    ok = q.try_pop(val);
    raise_if_false(ok);
    raise_if_false(val == 999);

    // Push some values...
    q.push(42);
    q.push(57);

    // ...close the queue
    q.close();

    // and check that we cannot push anything else
    try {
        q.try_push(47);
        raise_if_false(false);
    } catch (const ClosedQueue&) {
        raise_if_false(true);
    }

    // but we can pop until the queue gets empty
    val = q.pop();
    raise_if_false(val == 42);

    val = q.pop();
    raise_if_false(val == 57);
    try {
        q.try_pop(val);
        raise_if_false(false);
    } catch (const ClosedQueue&) {
        raise_if_false(true);
    }

    std::cout << "[OK] test_non_blocking_queue__complex\n";
}

struct Value {
    int i;
    Value(int i=0) : i(i) {}
    operator int() { return i; }
};

void test_non_blocking_queue__value() {
    Queue<Value> q(QUEUE_MAXSIZE);
    Value val;
    bool ok;

    // We expect to be able to push N elements for a N limit
    // queue without trouble
    for (int i = 0;  i < QUEUE_MAXSIZE; ++i) {
        ok = q.try_push(i);
        raise_if_false(ok);
    }

    // The N+1 element however, should fail
    ok = q.try_push(999);
    raise_if_false(!ok);

    // A pop should work just fine, retrieving
    // the first pushed element.
    ok = q.try_pop(val);
    raise_if_false(ok);
    raise_if_false(val == 0);

    // Now that we made room we can push one more element
    ok = q.try_push(999);
    raise_if_false(ok);

    // We expect to pop all the elements of the queue in
    // a FIFO order (in the loop we pop N-1 elements)
    for (int i = 1;  i < QUEUE_MAXSIZE; ++i) {
        ok = q.try_pop(val);
        raise_if_false(ok);
        raise_if_false(val == i);
    }

    // Pop the last pushed value
    ok = q.try_pop(val);
    raise_if_false(ok);
    raise_if_false(val == 999);

    // Push some values...
    q.push(42);
    q.push(57);

    // ...close the queue
    q.close();

    // and check that we cannot push anything else
    try {
        q.try_push(47);
        raise_if_false(false);
    } catch (const ClosedQueue&) {
        raise_if_false(true);
    }

    // but we can pop until the queue gets empty
    val = q.pop();
    raise_if_false(val == 42);

    val = q.pop();
    raise_if_false(val == 57);
    try {
        q.try_pop(val);
        raise_if_false(false);
    } catch (const ClosedQueue&) {
        raise_if_false(true);
    }

    std::cout << "[OK] test_non_blocking_queue__value\n";
}

void test_non_blocking_queue__ptr_void() {
    Queue<void*> q(QUEUE_MAXSIZE);
    void* val;
    bool ok;

    // We expect to be able to push N elements for a N limit
    // queue without trouble
    for (int i = 0;  i < QUEUE_MAXSIZE; ++i) {
        val = new Value(i);
        ok = q.try_push(val);
        raise_if_false(ok);
    }

    // The N+1 element however, should fail
    val = new Value(999);
    ok = q.try_push(val);
    raise_if_false(!ok);
    delete (Value*)val;

    // A pop should work just fine, retrieving
    // the first pushed element.
    ok = q.try_pop(val);
    raise_if_false(ok);
    raise_if_false(*(Value*)val == 0);
    delete (Value*)val;

    // Now that we made room we can push one more element
    val = new Value(999);
    ok = q.try_push(val);
    raise_if_false(ok);

    // We expect to pop all the elements of the queue in
    // a FIFO order (in the loop we pop N-1 elements)
    for (int i = 1;  i < QUEUE_MAXSIZE; ++i) {
        ok = q.try_pop(val);
        raise_if_false(ok);
        raise_if_false(*(Value*)val == i);
        delete (Value*)val;
    }

    // Pop the last pushed value
    ok = q.try_pop(val);
    raise_if_false(ok);
    raise_if_false(*(Value*)val == 999);
    delete (Value*)val;

    // Push some values...
    q.push(new Value(42));
    q.push(new Value(57));

    // ...close the queue
    q.close();

    // and check that we cannot push anything else
    val = new Value(47);
    try {
        q.try_push(val);
        raise_if_false(false);
    } catch (const ClosedQueue&) {
        raise_if_false(true);
    }
    delete (Value*)val;

    // but we can pop until the queue gets empty
    val = q.pop();
    raise_if_false(*(Value*)val == 42);
    delete (Value*)val;

    val = q.pop();
    raise_if_false(*(Value*)val == 57);
    delete (Value*)val;

    try {
        q.try_pop(val);
        raise_if_false(false);
    } catch (const ClosedQueue&) {
        raise_if_false(true);
    }

    std::cout << "[OK] test_non_blocking_queue__ptr_void\n";
}

void test_non_blocking_queue__ptr_value() {
    Queue<Value*> q(QUEUE_MAXSIZE);
    Value *val;
    bool ok;

    // We expect to be able to push N elements for a N limit
    // queue without trouble
    for (int i = 0;  i < QUEUE_MAXSIZE; ++i) {
        val = new Value(i);
        ok = q.try_push(val);
        raise_if_false(ok);
    }

    // The N+1 element however, should fail
    val = new Value(999);
    ok = q.try_push(val);
    raise_if_false(!ok);
    delete val;

    // A pop should work just fine, retrieving
    // the first pushed element.
    ok = q.try_pop(val);
    raise_if_false(ok);
    raise_if_false(*val == 0);
    delete val;

    // Now that we made room we can push one more element
    val = new Value(999);
    ok = q.try_push(val);
    raise_if_false(ok);

    // We expect to pop all the elements of the queue in
    // a FIFO order (in the loop we pop N-1 elements)
    for (int i = 1;  i < QUEUE_MAXSIZE; ++i) {
        ok = q.try_pop(val);
        raise_if_false(ok);
        raise_if_false(*val == i);
        delete val;
    }

    // Pop the last pushed value
    ok = q.try_pop(val);
    raise_if_false(ok);
    raise_if_false(*val == 999);
    delete val;

    // Push some values...
    q.push(new Value(42));
    q.push(new Value(57));

    // ...close the queue
    q.close();

    // and check that we cannot push anything else
    val = new Value(47);
    try {
        q.try_push(val);
        raise_if_false(false);
    } catch (const ClosedQueue&) {
        raise_if_false(true);
    }
    delete val;

    // but we can pop until the queue gets empty
    val = q.pop();
    raise_if_false(*val == 42);
    delete val;

    val = q.pop();
    raise_if_false(*val == 57);
    delete val;

    try {
        q.try_pop(val);
        raise_if_false(false);
    } catch (const ClosedQueue&) {
        raise_if_false(true);
    }

    std::cout << "[OK] test_non_blocking_queue__ptr_value\n";
}

int main() try {
    test_non_blocking_queue__int();
    test_non_blocking_queue__complex();
    test_non_blocking_queue__value();
    test_non_blocking_queue__ptr_void();
    test_non_blocking_queue__ptr_value();
    return 0;
} catch (const std::exception& err) {
    std::cout << "Exception: " << err.what() << "\n";
    return 1;
} catch (...) {
    std::cout << "Unknown exception\n";
    return 2;
}
