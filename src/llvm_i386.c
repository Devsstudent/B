#include " building.h"
// Lit tout un FILE* dans un buffer alloué dynamiquement. Retourne NULL en cas d'erreur.
// La taille lue est placée dans *out_len (optionnel).
static char *read_all(FILE *fp, size_t *out_len) {
    size_t cap = 64 * 1024;
    size_t len = 0;
    char *buf = (char *)malloc(cap);
    if (!buf) return NULL;

    for (;;) {
        if (len == cap) {
            size_t ncap = cap * 2;
            char *nbuf = (char *)realloc(buf, ncap);
            if (!nbuf) { free(buf); return NULL; }
            buf = nbuf; cap = ncap;
        }
        size_t n = fread(buf + len, 1, cap - len, fp);
        len += n;
        if (n == 0) {
            if (feof(fp)) break;
            if (ferror(fp)) { free(buf); return NULL; }
        }
    }
    // Ajoute un '\0' final pour usage texte
    if (len == cap) {
        char *nbuf = (char *)realloc(buf, cap + 1);
        if (!nbuf) { free(buf); return NULL; }
        buf = nbuf;
    }
    buf[len] = '\0';
    if (out_len) *out_len = len;
    return buf;
}

// Génère de l'ASM i386 en syntaxe Intel via llc et le renvoie en mémoire.
// Retourne NULL en cas d'erreur; écrit des messages sur stderr.
char *generate_asm_intel_with_llc(LLVMModuleRef mod, const char *triple) {
    if (!triple) triple = "i386-pc-linux-gnu";

    // 1) Exporter le module en IR texte dans un fichier temporaire
    char tmpl[] = "/tmp/llvm_ir_XXXXXX.ll";
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        fprintf(stderr, "mkstemp failed: %s\n", strerror(errno));
        return NULL;
    }

    char *ir = LLVMPrintModuleToString(mod); // à libérer via LLVMDisposeMessage
    if (!ir) {
        fprintf(stderr, "LLVMPrintModuleToString failed\n");
        close(fd);
        unlink(tmpl);
        return NULL;
    }

    size_t ir_len = strlen(ir);
    ssize_t wr = write(fd, ir, ir_len);
    LLVMDisposeMessage(ir);
    if (wr < 0 || (size_t)wr != ir_len) {
        fprintf(stderr, "write failed: %s\n", strerror(errno));
        close(fd);
        unlink(tmpl);
        return NULL;
    }
    close(fd);

    // 2) Appeler llc pour produire de l'ASM Intel sur stdout
    // -o - => sortie sur stdout
    // -x86-asm-syntax=intel => syntaxe Intel
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "llc -mtriple=%s -x86-asm-syntax=intel -o - \"%s\"",
             triple, tmpl);

    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        fprintf(stderr, "popen failed: %s\n", strerror(errno));
        unlink(tmpl);
        return NULL;
    }

    char *asm_text = read_all(pipe, NULL);
    int status = pclose(pipe);

    // 3) Nettoyage du fichier temporaire
    unlink(tmpl);

    if (status != 0) {
        fprintf(stderr, "llc returned non-zero status (%d). Output:\n%s\n",
                status, asm_text ? asm_text : "(null)");
        free(asm_text);
        return NULL;
    }

    if (!asm_text || asm_text[0] == '\0') {
        fprintf(stderr, "llc produced no output\n");
        free(asm_text);
        return NULL;
    }

    return asm_text; // à free() par l'appelant
}

// Exemple d’usage
void generate_asm_with_llc_example(LLVMModuleRef mod) {
    // Assurez-vous d’avoir initialisé les cibles LLVM quelque part:
    // LLVMInitializeX86Target();
    // LLVMInitializeX86TargetInfo();
    // LLVMInitializeX86TargetMC();
    // LLVMInitializeX86AsmPrinter();

    LLVMSetTarget(mod, "i386-pc-linux-gnu");

    char *asm_intel = generate_asm_intel_with_llc(mod, "i386-pc-linux-gnu");
    if (!asm_intel) {
        fprintf(stderr, "Failed to generate Intel ASM via llc\n");
        return;
    }

    printf("=== ASM (Intel) ===\n%s\n", asm_intel);
    free(asm_intel);
}
