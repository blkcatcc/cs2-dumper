#include "dumper.h"

#include <elf.h>

static u16 rd16(const u8 *b, size_t off) {
    u16 v;
    memcpy(&v, b + off, 2);
    return v;
}

static u32 rd32(const u8 *b, size_t off) {
    u32 v;
    memcpy(&v, b + off, 4);
    return v;
}

static u64 rd64(const u8 *b, size_t off) {
    u64 v;
    memcpy(&v, b + off, 8);
    return v;
}

/* Static initialisers live in .rela.dyn, not in the file image. */
static void apply_relocations(Image *img, const u8 *file, size_t file_len) {
    if (file_len < sizeof(Elf64_Ehdr)) return;
    const Elf64_Ehdr *eh = (const Elf64_Ehdr *)file;
    if (memcmp(eh->e_ident, ELFMAG, SELFMAG) != 0) return;
    if (eh->e_machine != EM_X86_64) return;

    for (int i = 0; i < eh->e_phnum; i++) {
        size_t ph = (size_t)eh->e_phoff + (size_t)i * eh->e_phentsize;
        if (ph + sizeof(Elf64_Phdr) > file_len) return;
        const Elf64_Phdr *p = (const Elf64_Phdr *)(file + ph);
        if (p->p_type != PT_DYNAMIC) continue;

        size_t dyn = (size_t)p->p_offset, dyn_sz = (size_t)p->p_filesz;
        if (dyn + dyn_sz > file_len) return;

        u64 rela_addr = 0, rela_sz = 0, rela_ent = sizeof(Elf64_Rela);
        for (size_t e = dyn; e + 16 <= dyn + dyn_sz; e += 16) {
            i64 tag;
            u64 val;
            memcpy(&tag, file + e, 8);
            memcpy(&val, file + e + 8, 8);
            if (tag == DT_NULL) break;
            if (tag == DT_RELA) rela_addr = val;
            else if (tag == DT_RELASZ) rela_sz = val;
            else if (tag == DT_RELAENT) rela_ent = val;
        }
        if (!rela_addr || !rela_sz || rela_ent == 0) return;

        i64 rela_off = image_vaddr_to_off(img, rela_addr);
        if (rela_off < 0) return;

        size_t count = (size_t)(rela_sz / rela_ent);
        for (size_t r = 0; r < count; r++) {
            size_t at = (size_t)rela_off + r * (size_t)rela_ent;
            if (at + sizeof(Elf64_Rela) > img->len) break;
            Elf64_Rela rel;
            memcpy(&rel, img->data + at, sizeof(rel));
            if (ELF64_R_TYPE(rel.r_info) != R_X86_64_RELATIVE) continue;
            if ((u64)rel.r_offset + 8 > img->len) continue;
            u64 value = rel.r_addend;
            memcpy(img->data + rel.r_offset, &value, 8);
        }
        return;
    }
}

int image_load(Image *img, const char *path) {
    memset(img, 0, sizeof(*img));
    img->path = xstrdup(path);

    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    long fsz = ftell(f);
    if (fsz <= 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    u8 *file = xmalloc((size_t)fsz);
    size_t got = fread(file, 1, (size_t)fsz, f);
    fclose(f);

    if (got < sizeof(Elf64_Ehdr) || memcmp(file, ELFMAG, SELFMAG) != 0 ||
        file[EI_CLASS] != ELFCLASS64) {
        free(file);
        return 0;
    }

    const Elf64_Ehdr *eh = (const Elf64_Ehdr *)file;
    img->segs = xcalloc(eh->e_phnum ? eh->e_phnum : 1, sizeof(*img->segs));

    u64 end = 0;
    for (int i = 0; i < eh->e_phnum; i++) {
        size_t ph = (size_t)eh->e_phoff + (size_t)i * eh->e_phentsize;
        if (ph + sizeof(Elf64_Phdr) > got) break;
        const Elf64_Phdr *p = (const Elf64_Phdr *)(file + ph);
        if (p->p_type != PT_LOAD) continue;
        Seg *s = &img->segs[img->nsegs++];
        s->off = p->p_offset;
        s->vaddr = p->p_vaddr;
        s->filesz = p->p_filesz;
        s->memsz = p->p_memsz;
        if (p->p_vaddr + p->p_memsz > end) end = p->p_vaddr + p->p_memsz;
    }
    if (end == 0) {
        free(file);
        image_free(img);
        return 0;
    }

    img->len = (size_t)end;
    img->data = xcalloc(img->len, 1);
    for (size_t i = 0; i < img->nsegs; i++) {
        const Seg *s = &img->segs[i];
        if (s->off + s->filesz > got) continue;
        if (s->vaddr + s->filesz > img->len) continue;
        memcpy(img->data + s->vaddr, file + s->off, (size_t)s->filesz);
    }

    apply_relocations(img, file, got);
    free(file);
    return 1;
}

void image_free(Image *img) {
    free(img->data);
    free(img->segs);
    free(img->path);
    img->data = NULL;
    img->segs = NULL;
    img->path = NULL;
    img->len = img->nsegs = 0;
}

u64 image_off_to_vaddr(const Image *img, size_t off) {
    for (size_t i = 0; i < img->nsegs; i++) {
        const Seg *s = &img->segs[i];
        if (off >= s->off && off < s->off + s->filesz) return s->vaddr + (off - s->off);
    }
    return off;
}

i64 image_vaddr_to_off(const Image *img, u64 vaddr) {
    for (size_t i = 0; i < img->nsegs; i++) {
        const Seg *s = &img->segs[i];
        if (vaddr >= s->vaddr && vaddr < s->vaddr + s->memsz)
            return (i64)(s->off + (vaddr - s->vaddr));
    }
    return -1;
}

/* Module-relative address of an exported symbol, or 0.

   Everything is indexed by virtual address, matching how the module appears in
   process memory. glibc relocates .dynamic in place, so DT_SYMTAB/DT_STRTAB hold
   absolute addresses in a live image and module-relative ones on disk. */
u64 image_export(const Image *img, const char *name) {
    const u8 *b = img->data;
    size_t len = img->len;
    if (len < sizeof(Elf64_Ehdr)) return 0;
    if (memcmp(b, ELFMAG, SELFMAG) != 0 || b[EI_CLASS] != ELFCLASS64) return 0;

    size_t e_phoff = (size_t)rd64(b, 0x20);
    size_t e_phentsize = rd16(b, 0x36);
    size_t e_phnum = rd16(b, 0x38);

    u64 dyn_addr = 0, dyn_size = 0;
    for (size_t i = 0; i < e_phnum; i++) {
        size_t ph = e_phoff + i * e_phentsize;
        if (ph + 0x28 > len) return 0;
        if (rd32(b, ph) == PT_DYNAMIC) {
            dyn_addr = rd64(b, ph + 0x10);
            dyn_size = rd64(b, ph + 0x20);
            break;
        }
    }
    if (dyn_size == 0) return 0;

    u64 symtab = 0, strtab = 0, syment = 24;
    for (u64 e = dyn_addr; e + 16 <= dyn_addr + dyn_size; e += 16) {
        if (e + 16 > len) return 0;
        i64 tag;
        u64 val;
        memcpy(&tag, b + e, 8);
        memcpy(&val, b + e + 8, 8);
        if (tag == DT_NULL) break;
        if (tag == DT_STRTAB) strtab = val;
        else if (tag == DT_SYMTAB) symtab = val;
        else if (tag == DT_SYMENT) syment = val;
    }
    if (!symtab || !strtab || syment == 0) return 0;

    if (img->base) {
        if (symtab >= img->base) symtab -= img->base;
        if (strtab >= img->base) strtab -= img->base;
    }
    if (strtab <= symtab) return 0;

    /* No explicit count in .dynamic; the table runs up to the string table. */
    u64 count = (strtab - symtab) / syment;
    for (u64 i = 0; i < count; i++) {
        u64 sym = symtab + i * syment;
        if (sym + 24 > len) return 0;
        u32 st_name = rd32(b, sym);
        u16 st_shndx = rd16(b, sym + 6);
        u64 st_value = rd64(b, sym + 8);
        if (st_shndx == SHN_UNDEF) continue;
        if (strtab + st_name >= len) continue;
        if (strcmp((const char *)b + strtab + st_name, name) == 0) return st_value;
    }
    return 0;
}
