#include <cstdint>
#include <cstdio>
#include <iostream>

    constexpr uint8_t get_opcode(uint16_t instruction) {
        return instruction & 0xF;
    };
    constexpr uint16_t get_operand(uint16_t instruction, char i)
    {
        return (instruction >> (4 + 6*i)) & 0x3F;
    }
    constexpr uint16_t get_reg(uint16_t operand) {
        return operand & 7;
    }
    constexpr uint8_t advance_pc(uint16_t operand)
    {
        return (operand >= 0x10 && operand <= 0x17) || (operand == 0x1E || operand == 0x1F);
    }
class DCPU
{
    enum Opcodes
    {
        NON_BASIC,
        SET,
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
        SHL,
        SHR,
        AND,
        BOR,
        XOR,
        IFE,
        IFN,
        IFG,
        IFB
    };

    enum NonBasicOpcodes
    {
        JSR = 0x01
    };


    public:
    uint16_t    r[8],
                pc,
                sp,
                o,
                m[0x10000];

    DCPU() :m({0x7c01, 0x0030}) {}



    uint16_t *GetValue(uint16_t operand)
    {

        //  0x00-0x07: register (A, B, C, X, Y, Z, I or J, in that order)
        if (operand <= 0x7)
            return r + get_reg(operand);
        // 0x08-0x0f: [register]
        else if (operand <= 0xf)
            return m + r[get_reg(operand)];
        // 0x10-0x17: [next word + register]
        else if (operand <= 0x17)
            return m + ((r[get_reg(operand)] + m[pc++]) & 0xFFFF);
        else switch (operand)
        {
            case 0x18:  return m + sp++;    // POP / [SP++]
            case 0x19:  return m + sp;      // PEEK [SP]
            case 0x1a:  return m + (--sp);  // PUSH [--SP]
            case 0x1b:  return &sp;         // SP
            case 0x1c:  return &pc;         // PC
            case 0x1d:  return &o;          // O
            case 0x1e:  return m + m[pc++]; // [next word]
            case 0x1f:  return m + pc++;    // next word (literal)
            default:    return m + 0x3f - operand; // literal value 0x00-0x1f (literal)
        }

    }

    // some operand forms adbance the PC,
    void Skip()
    {
        uint16_t instruction = m[pc++];
        uint16_t dest, src;
        uint8_t opcode = get_opcode(instruction);
        pc += advance_pc(get_operand(instruction, 1));

        if (opcode == NON_BASIC)
        {
        }


    }

    void Step()
    {
        uint16_t    instruction = m[pc++],
                    a, b, *ap,
                    dest, src;
        uint32_t    res;

        uint8_t opcode = get_opcode(instruction);

        // aaaaaaaoooooo0000
        if (opcode == NON_BASIC) 
        {
            a = *GetValue(get_operand(instruction, 1));
            opcode = get_operand(instruction, 0);

            switch (opcode)
            {
                case JSR: // JSR - pushes the address of the next instruction to the stack, then sets PC to a
                    m[--sp] = pc;
                    pc = a;
                // case 0x02-0x3f: reserved
                default:
                   std::cerr << "Opcode not implemented yet!";
            }
        }
        // bbbbbbaaaaaaoooo
        else
        {
            dest = get_operand(instruction, 0);
            src  = get_operand(instruction, 1);
            ap = GetValue(dest);
            a  = *ap;
            b  = *GetValue(src);
            
            switch (opcode)
            {
                case SET: res = b; break;
                case ADD: res = a + b; break;
                case SUB: res = a - b; break;
                case MUL: res = a * b; break;
                case DIV: res = b ? a / b : 0; break;
                case MOD: res = b ? a % b : 0; break;
                case SHL: res = a << b; break;
                case SHR: res = a >> b; break;
                case AND: res = a & b; break;
                case BOR: res = a | b; break;
                case XOR: res = a ^ b; break;
                case IFE: if (a!=b) Skip(); break;
                case IFN: if (a==b) Skip(); break;
                case IFG: if (a<=b) Skip(); break;
                case IFB: if ((a&b)==0) Skip(); break;
            }
            // if a (dest) isnt a literal value, set a to the result of a calculation
            if (dest < 0x1F)
                *ap = res;
            return;
        }


    }
};

int main()
{
    DCPU cpu;
    cpu.Step();
    std::cout << cpu.r[0];
}
