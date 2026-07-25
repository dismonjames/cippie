# Cippie — Modern C++23 Build System and Package Manager

[![License: GPL-3.0-or-later](https://img.shields.io/badge/License-GPL_v3-blue.svg)](LICENSE)
[![Status: v0.1.0](https://img.shields.io/badge/Status-v0.1.0-green.svg)](docs/RELEASE_NOTES_0_1_0.md)

**Cippie** là build system, project manager và package manager hiện đại cho C++23, không cần CMake wrapper.

---

## Cài đặt nhanh

### Cách 1 — Tải binary chính thức (khuyến nghị)

```bash
curl -fsSL https://raw.githubusercontent.com/dismonjames/cippie/main/scripts/install.sh | bash
```

Hoặc kiểm tra script trước khi chạy:

```bash
curl -fsSL https://raw.githubusercontent.com/dismonjames/cippie/main/scripts/install.sh -o install-cippie.sh
less install-cippie.sh
bash install-cippie.sh
```

Binary được cài vào `$HOME/.local/bin/cippie` theo mặc định.

**Tùy chọn:**

```bash
# Chọn phiên bản cụ thể
bash install-cippie.sh --version 0.1.0

# Đặt prefix khác
bash install-cippie.sh --prefix "$HOME/.local"

# Ghi đè bản đang có
bash install-cippie.sh --force
```

### Cách 2 — Build từ source

```bash
git clone https://github.com/dismonjames/cippie.git
cd cippie
./scripts/bootstrap.sh --prefix "$HOME/.local"
```

Script bootstrap tự động:
1. Build Cippie tạm thời bằng CMake.
2. Dùng Cippie vừa build để tự biên dịch lại (Gen1).
3. Chạy toàn bộ test suite.
4. Cài binary đã tự host vào prefix.

---

## Bắt đầu dự án mới

```bash
cippie new hello
cd hello
cippie build
cippie run
cippie test
```

---

## Gỡ cài đặt

```bash
curl -fsSL https://raw.githubusercontent.com/dismonjames/cippie/main/scripts/uninstall.sh | bash
```

Hoặc nếu đã clone repo:

```bash
./scripts/uninstall.sh --prefix "$HOME/.local"
```

> Config và package cache (`~/.cache/cippie`, `~/.config/cippie`) được giữ nguyên.
> Để xóa thủ công: `rm -rf ~/.cache/cippie ~/.config/cippie`

---

## Cài PATH (nếu cần)

Nếu lệnh `cippie` chưa nhận ra sau khi cài, thêm vào `~/.bashrc` hoặc `~/.zshrc`:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

---

## Workflow phát triển Cippie

Cippie tự build bằng chính mình:

```bash
cippie build       # build Cippie bằng Cippie
cippie test        # chạy test suite
./scripts/check.sh # kiểm tra toàn bộ: source sync, build, test, self-host 2-gen
```

> CMake chỉ được dùng để bootstrap lần đầu hoặc khôi phục khi self-host bị lỗi.

---

## Cập nhật

```bash
sh install-cippie.sh --version <phiên-bản-mới> --force
```

---

## License

[GNU General Public License v3.0 or later](LICENSE)
