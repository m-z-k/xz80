#include "xz80.hpp"

class Hello : public Xz80::Generator {
  void syscall(uint8_t id) {
    ld(C, id);
    call(0x0005);
  }

 public:
  Hello() : Xz80::Generator(0x0100) {
    ld(IX, "MSG_ADDR");
    ld(E, IX(0));
    ld(D, IX(1));
    syscall(9);
    ret();

    l("MSG");
    db("Hello MSX world!\n");
    db('$');

    l("MSG_ADDR");
    dw("MSG");
  }
};

int main(void) {
  Hello h;
  h.resolve(true);
  h.dump();
  h.save("HELLO2.COM");
}