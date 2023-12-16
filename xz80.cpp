#include "xz80.hpp"

class Gen : public Xz80::Generator {
 public:
  Gen() : Xz80::Generator(0x0100) {}
  void testcase(void) {
    // 内部処理用クラスのテスト
    Xz80::Formatter::Formatter f("漢字ir,r,r,r,r,r,r,r,r,");
    std::cout << (f % ";" % A % C() % HL % HL() % BC() % DE() % IX() % SP() % IY(0)).str() << std::endl;

    Xz80::Formatter::Formatter g("idxdx");
    std::cout << (g % ";" % 42 % 42 % -1 % -1).str() << std::endl;

    Xz80::Formatter::Formatter h("ib,w");
    std::initializer_list<uint8_t> bs{0xde, 0xad, 0xbe, 0xef};
    std::initializer_list<uint16_t> ws{0xdead, 0xbeef, 0xcafe, 0xbabe};
    std::cout << (h % ";" % bs % ws).str() << std::endl;

    l("HEAD");
    ld(A, B);
    ld(C, 64);
    ld(A, HL());
    ld(A, IX(1));
    ld(B, IY(-1));
    ld(C, IX(127));
    ld(D, IY(-128));
    ld(H, IX(0));
    ld(L, IY(0));
    ld(HL(), D);
    ld(IX(3), H);
    ld(IY(-3), L);
    ld(HL(), 16);
    ld(IX(8), 255);
    ld(IY(-8), 0);
    ld(A, BC());
    ld(A, DE());
    ld(A, mem(0xbeef));
    ld(A, mem("HEAD"));
    ld(A, mem(0xbeef));
    ld(BC(), A);
    ld(DE(), A);
    ld(mem(0xcafe), A);
    ld(A, I);
    ld(I, A);
    ld(A, R);
    ld(R, A);
    // ----
    ld(BC, 0x0123);
    ld(DE, 0x4567);
    ld(HL, 0x89ab);
    ld(SP, 0xcdef);
    ld(IX, 0xcafe);
    ld(IY, 0xbabe);
    ld(BC, "HEAD");
    ld(DE, "HEAD");
    ld(HL, "HEAD");
    ld(SP, "HEAD");
    ld(IX, "HEAD");
    ld(IY, "HEAD");

    ld(HL, mem(0x0123));
    ld(BC, mem(0x4567));
    ld(DE, mem(0x89ab));
    ld(SP, mem(0xcdef));
    ld(IX, mem(0xcc55));
    ld(IY, mem(0xaa33));
    ld(HL, mem("HEAD"));
    ld(BC, mem("HEAD"));
    ld(DE, mem("HEAD"));
    ld(SP, mem("HEAD"));
    ld(IX, mem("HEAD"));
    ld(IY, mem("HEAD"));

    ld(mem(0xcdef), HL);
    ld(mem(0x0123), BC);
    ld(mem(0x4567), DE);
    ld(mem(0x89ab), SP);
    ld(mem(0x1133), IX);
    ld(mem(0x77ff), IY);
    ld(mem("HEAD"), HL);
    ld(mem("HEAD"), BC);
    ld(mem("HEAD"), DE);
    ld(mem("HEAD"), SP);
    ld(mem("HEAD"), IX);
    ld(mem("HEAD"), IY);

    ld(SP, HL);
    ld(SP, IX);
    ld(SP, IY);
    // ----
    ldi();
    ldir();
    ldd();
    lddr();
    // ----
    ex(DE, HL);
    ex(AF, AF);  // EX AF, AF'
    exx();
    ex(SP(), HL);
    ex(SP(), IX);
    ex(SP(), IY);
    // ----
    push(BC);
    push(DE);
    push(HL);
    push(AF);
    push(IX);
    push(IY);

    pop(BC);
    pop(DE);
    pop(HL);
    pop(AF);
    pop(IX);
    pop(IY);
    // ----
    rlca();
    rla();
    rlc(A);
    rlc(B);
    rlc(C);
    rlc(D);
    rlc(E);
    rlc(H);
    rlc(L);
    rlc(HL());
    rlc(IX(-7));
    rlc(IY(42));
    rl(A);
    rl(B);
    rl(C);
    rl(D);
    rl(E);
    rl(H);
    rl(L);
    rl(HL());
    rl(IX(127));
    rl(IX(-128));
    // ----
    rrca();
    rra();
    rrc(A);
    rrc(B);
    rrc(C);
    rrc(D);
    rrc(E);
    rrc(H);
    rrc(L);
    rrc(HL());
    rrc(IX(-7));
    rrc(IY(42));
    rr(A);
    rr(B);
    rr(C);
    rr(D);
    rr(E);
    rr(H);
    rr(L);
    rr(HL());
    rr(IX(127));
    rr(IX(-128));
    // ----
    sla(A);
    sla(B);
    sla(C);
    sla(D);
    sla(E);
    sla(H);
    sla(L);
    sla(HL());
    sla(IX(65));
    sla(IY(-1));
    // ----
    sra(A);
    sra(B);
    sra(C);
    sra(D);
    sra(E);
    sra(H);
    sra(L);
    sra(HL());
    sra(IX(65));
    sra(IY(-1));
    // ----
    srl(A);
    srl(B);
    srl(C);
    srl(D);
    srl(E);
    srl(H);
    srl(L);
    srl(HL());
    srl(IX(65));
    srl(IY(-1));
    // ----
    add(A, A);
    add(A, B);
    add(A, C);
    add(A, D);
    add(A, E);
    add(A, H);
    add(A, L);
    add(A, 100);
    add(A, HL());
    add(A, IX(32));
    add(A, IY(-32));
    // ----
    adc(A, A);
    adc(A, B);
    adc(A, C);
    adc(A, D);
    adc(A, E);
    adc(A, H);
    adc(A, L);
    adc(A, 100);
    adc(A, HL());
    adc(A, IX(32));
    adc(A, IY(-32));
    // ----
    inc(A);
    inc(B);
    inc(C);
    inc(D);
    inc(E);
    inc(H);
    inc(L);
    inc(HL());
    inc(IX(50));
    inc(IY(-50));
    // ----
    sub(A, A);
    sub(A, B);
    sub(A, C);
    sub(A, D);
    sub(A, E);
    sub(A, H);
    sub(A, L);
    sub(A, 100);
    sub(A, HL());
    sub(A, IX(32));
    sub(A, IY(-32));
    // ----
    sbc(A, A);
    sbc(A, B);
    sbc(A, C);
    sbc(A, D);
    sbc(A, E);
    sbc(A, H);
    sbc(A, L);
    sbc(A, 100);
    sbc(A, HL());
    sbc(A, IX(32));
    sbc(A, IY(-32));
    // ----
    dec(A);
    dec(B);
    dec(C);
    dec(D);
    dec(E);
    dec(H);
    dec(L);
    dec(HL());
    dec(IX(50));
    dec(IY(-50));
    // ----
    add(HL, HL);
    add(HL, BC);
    add(HL, DE);
    add(HL, SP);
    adc(HL, HL);
    adc(HL, BC);
    adc(HL, DE);
    adc(HL, SP);
    add(IX, BC);
    add(IY, DE);
    add(IY, SP);
    inc(HL);
    inc(BC);
    inc(DE);
    inc(SP);
    inc(IX);
    inc(IY);
    sbc(HL, BC);
    sbc(HL, DE);
    sbc(HL, HL);
    sbc(HL, SP);
    dec(HL);
    dec(BC);
    dec(DE);
    dec(SP);
    dec(IX);
    dec(IY);
    // ----
    and(A);
    and(B);
    and(C);
    and(D);
    and(E);
    and(H);
    and(L);
    and(32);
    and(HL());
    and(IX(3));
    and(IY(-3));
    or (A);
    or (B);
    or (C);
    or (D);
    or (E);
    or (H);
    or (L);
    or (32);
    or (HL());
    or (IX(3));
    or (IY(-3));
    xor(A);
    xor(B);
    xor(C);
    xor(D);
    xor(E);
    xor(H);
    xor(L);
    xor(32);
    xor(HL());
    xor(IX(3));
    xor(IY(-3));
    cpl();
    neg();
    // ----
    ccf();
    scf();
    bit(0, A);
    bit(1, B);
    bit(2, C);
    bit(3, D);
    bit(4, E);
    bit(6, H);
    bit(7, L);
    bit(3, HL());
    bit(3, IX(10));
    bit(3, IY(-10));
    set(0, A);
    set(1, B);
    set(2, C);
    set(3, D);
    set(4, E);
    set(6, H);
    set(7, L);
    set(3, HL());
    set(3, IX(10));
    set(3, IY(-10));
    res(0, A);
    res(1, B);
    res(2, C);
    res(3, D);
    res(4, E);
    res(6, H);
    res(7, L);
    res(3, HL());
    res(3, IX(10));
    res(3, IY(-10));
    // ----
    cpi();
    cpir();
    cpd();
    cpdr();
    cp(A);
    cp(B);
    cp(C);
    cp(D);
    cp(E);
    cp(H);
    cp(L);
    cp(0);
    cp(255);
    cp(HL());
    cp(IX(0));
    cp(IY(-42));
    // ----
    l("JP0");
    jp(0xbeef);
    l("JP1");
    jp(NZ, 0x0123);
    jp(Z, 0x4567);
    jp(NC, 0x89ab);
    jp(Cy, 0xcdef);
    jp("JP0");
    jp(NZ, "JP1");
    jp(Z, "JP1");
    jp(NC, "JP1");
    jp(Cy, "JP1");

    jr(-126);  // この辺もラベルに対応させる
    jr(-1);
    jr(0);
    jr(1);
    jr(2);
    jr(129);
    jr("JR0");
    jr("JR0");
    l("JR0");
    jr("JR0");
    jr("JR0");

    jr(Cy, -100);
    jr(NC, -50);
    jr(Z, 50);
    jr(NZ, 100);
    jr(Cy, "JR0");

    jp(HL());
    jp(IX());
    jp(IY());
    djnz(-126);
    djnz(0);
    djnz(129);
    l("DJNZ0");
    djnz("DJNZ0");
    l("CALL0");
    call(0xcafe);
    l("CALL1");
    call(NZ, 0x0123);
    call(Z, 0x4567);
    call(NC, 0x89ab);
    call(Cy, 0xcdef);
    call(PO, 0x1133);
    call(PE, 0x5577);
    call(P, 0xaabb);
    call(M, 0xccdd);

    call("CALL0");
    call(NZ, "CALL1");
    call(Z, "CALL1");
    call(NC, "CALL1");
    call(Cy, "CALL1");
    call(PO, "CALL1");
    call(PE, "CALL1");
    call(P, "CALL1");
    call(M, "CALL1");

    ret();
    ret(NZ);
    ret(Z);
    ret(NC);
    ret(Cy);
    ret(PO);
    ret(PE);
    ret(P);
    ret(M);
    reti();
    retn();
    for (int i = 0; i < 64; i += 8) {
      rst(i);
    }
    // ----
    nop();
    halt();
    di();
    ei();
    im(0);
    im(1);
    im(2);
    // ----
    in(A, io(0x12));
    in(A, C());
    in(B, C());
    in(C, C());
    in(D, C());
    in(E, C());
    in(H, C());
    in(L, C());
    ini();
    inir();
    ind();
    indr();
    // ----
    out(io(0x12), A);
    out(C(), A);
    out(C(), B);
    out(C(), C);
    out(C(), D);
    out(C(), E);
    out(C(), H);
    out(C(), L);
    outi();
    otir();
    outd();
    otdr();
    // ----
    daa();
    rld();
    rrd();
    // ----
    l("LABEL1");
    dw(0x55aa);
    l("LABEL2");
    dw("LABEL1");
    l("LABEL3");
    dw({"LABEL1", "LABEL2", "LABEL3"});
    db({0xcd, 0xf7, 0xf8});
    dw({0x1234, 0x5678, 0xcc33});
    db("Hello!\r\n");
    // ----
  }
};

int main(void) {
  Gen g;
  g.testcase();
  g.resolve(true);
  g.dump();

  g.save("exp.bin");
}