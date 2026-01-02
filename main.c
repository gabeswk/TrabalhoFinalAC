/*
 * =================================================================================
 * TRABALHO FINAL - ARQUITETURA DE COMPUTADORES (SIMULADOR RISC 16-BIT)
 * =================================================================================
 *
 * ESPECIFICAÇÕES GERAIS:
 * 1. Palavra de dados/instrução: 16 bits.
 * 2. Memória: 16KB (endereços 0x0000 a 0x2000). Mapeada como array de 8192 posições.
 * 3. Registradores: 16 (R0-R15).
 * - R14 = SP (Stack Pointer) -> Inicia em 0x2000.
 * - R15 = PC (Program Counter).
 * 4. Flags: Zero (Z) e Carry (C). Apenas instruções da ULA modificam as flags.
 *
 * =================================================================================
 * TODO LIST (O QUE PRECISA SER FEITO):
 * =================================================================================
 *
 * [ ] 1. ESTRUTURAS DE DADOS (STRUCTS)
 * - Criar struct 'CPU' contendo:
 * - uint16_t reg[16];        // R0 a R15
 * - uint16_t memory[8192];   // Memória RAM (0x0000 - 0x2000)
 * - uint16_t ir;             // Instruction Register
 * - int flag_z;              // Flag Zero
 * - int flag_c;              // Flag Carry
 * - bool mem_access[8192];   // (Dica PDF) Para rastrear posições acessadas p/ o print final.
 *
 * [ ] 2. LEITURA DO ARQUIVO (LOADER)
 * - Abrir arquivo .txt ou .hex passado por argumento.
 * - Ler linhas no formato "<endereço> <conteúdo>" (hexadecimal).
 * - Preencher o array 'memory' da CPU com esses valores.
 * - Armazenar breakpoints (se houver, passados via argv).
 *
 * [ ] 3. CICLO DE EXECUÇÃO (Loop Principal)
 * - Loop while(1) até encontrar instrução HALT.
 * - 3.1. BUSCA (FETCH):
 * IR = memory[PC]
 * old_pc = PC;  // Guardar p/ debug/breakpoints
 * PC = PC + 1;  // IMPORTANTE: PC incrementa ANTES de executar [PDF Pag 5]
 *
 * - 3.2. DECODIFICAÇÃO (DECODE):
 * Extrair Opcode (bits 0-3).
 * Extrair Operandos (Rd, Rm, Rn, Imediato) dependendo do tipo da instrução.
 * Dica: Fazer extensão de sinal para imediatos em JMP, J<cond> e MOV [PDF Pag 5].
 *
 * - 3.3. EXECUÇÃO (EXECUTE) - SWITCH CASE NO OPCODE:
 * - JUMP (JMP, JEQ, JNE, JLT, JGE): Verificar flags Z/C e somar PC + Imediato.
 * - MEMÓRIA (LDR, STR):
 * * Atenção: STR salva na memória, LDR carrega p/ registrador.
 * * IMPORTANTE: Verificar Mapeamento de E/S (Endereços >= 0xF000).
 * - 0xF000/0xF001: Char I/O (getchar/putchar).
 * - 0xF002/0xF003: Int I/O (scanf/printf).
 * - MOVIMENTAÇÃO (MOV): Rd = Imediato.
 * - ULA (ADD, SUB, AND, OR, SHL, SHR, CMP):
 * * Realizar operação.
 * * ATUALIZAR FLAGS Z e C (apenas aqui!).
 * * CMP é um SUB que não salva resultado, só flags.
 * - PILHA (PUSH, POP):
 * * PUSH: SP--, Mem[SP] = Rn.
 * * POP: Rd = Mem[SP], SP++.
 * - CONTROLE (HALT): Break no loop.
 *
 * [ ] 4. SISTEMA DE SAÍDA (DUMP)
 * - Criar função para imprimir o estado da máquina (chamada no HALT ou Breakpoint).
 * - Imprimir R0-R15 em Hexa.
 * - Imprimir Flags Z e C.
 * - Imprimir conteúdo da Memória de Dados (apenas posições marcadas como acessadas).
 * - Imprimir Pilha (se SP != 0x2000, imprimir de SP até o fundo).
 *
 * [ ] 5. IMPLEMENTAR FUNÇÕES AUXILIARES (Dicas do PDF)
 * - Extensão de sinal (converter 6 bits ou 8 bits para 16 bits signed).
 * - Verificação de breakpoints.
 *
 * =================================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE 8192
#define NUM_REGS 16

//struct to cpu

typedef struct {
    uint16_t reg[NUM_REGS];       // R0 a R15
    uint16_t memory[MEMORY_SIZE]; // Memória RAM (0x0000 - 0x2000)
    uint16_t ir;                  // Instruction Register
    int flag_z;                   // Flag Zero
    int flag_c;                   // Flag Carry
    bool mem_access[MEMORY_SIZE]; // Para rastrear posições acessadas p/ o print final
} CPU;

//init cpu function

void cpu_init(CPU *cpu) {
    for (int i = 0; i < NUM_REGS; i++) {
        cpu->reg[i] = 0;
    }
    for (int i = 0; i < MEMORY_SIZE; i++) {
        cpu->memory[i] = 0;
        cpu->mem_access[i] = false;
    }
    cpu->ir = 0;
    cpu->flag_z = 0;
    cpu->flag_c = 0;
    cpu->reg[14] = 0x2000; // Initialize SP
    cpu->reg[15] = 0;      // Initialize PC
}

//Loader function

void load_program(CPU *cpu, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    uint16_t address, content;
    while (fscanf(f, "%hx %hx", &address, &content) == 2) {
        if (address < MEMORY_SIZE) {
            cpu->memory[address] = content;
        }
    }

    fclose(f);
}

//cicle FETCH ONLY SEARCH INSTRUCTION
void cpu_run(CPU *cpu) {
    while (1) {
        uint16_t pc = cpu->reg[15];

        cpu->ir = cpu->memory[pc];
        cpu->mem_access[pc] = true;

        cpu->reg[15]++; // PC incrementa antes da execucao

        uint16_t opcode = cpu->ir & 0x000F;

        switch (opcode) {
            case 0x0: // HALT (exemplo)
                printf("HALT encontrado\n");
                return;

            default:
                printf("Opcode nao implementado: 0x%X\n", opcode);
                return;
        }
    }
}

//main function

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s programa.txt\n", argv[0]);
        return 1;
    }

    CPU cpu;
    cpu_init(&cpu);
    load_program(&cpu, argv[1]);
    cpu_run(&cpu);

    return 0;
}
