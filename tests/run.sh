#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
BIN="${BIN:-$ROOT/../myfind}"

fail() { echo "FAIL: $1" >&2; exit 1; }

normalize_stdout() {

  sort
}

run_ok () {
  local name="$1"; shift
  local expected_out="$ROOT/expected/${name}.out"
  local expected_err="$ROOT/expected/${name}.err"

  local tmp
  tmp="$(mktemp -d)"

  local out
  out="$(mktemp)"
  local err
  err="$(mktemp)"

  trap 'rm -rf "$tmp"; rm -f "$out" "$err"' RETURN

  cp -a "$ROOT/fixtures/basic/." "$tmp/"

  set +e
  (cd "$tmp" && "$BIN" "$@" > >(normalize_stdout > "$out") 2> "$err")
  local rc=$?
  set -e

  [[ $rc -eq 0 ]] || fail "$name (expected exit 0, got $rc)"
  diff -u "$expected_out" "$out" || fail "$name (stdout mismatch)"
  diff -u "$expected_err" "$err" || fail "$name (stderr mismatch)"

  echo "OK: $name"
}

run_fail () {
  local name="$1"; shift
  local expected_rc="$1"; shift
  local expected_out="$ROOT/expected/${name}.out"
  local expected_err="$ROOT/expected/${name}.err"

  local tmp
  tmp="$(mktemp -d)"

  local out
  out="$(mktemp)"
  local err
  err="$(mktemp)"

  trap 'rm -rf "$tmp"; rm -f "$out" "$err"' RETURN

  cp -a "$ROOT/fixtures/basic/." "$tmp/"

  set +e
  (cd "$tmp" && "$BIN" "$@" > >(normalize_stdout > "$out") 2> "$err")
  local rc=$?
  set -e

  [[ $rc -eq $expected_rc ]] || fail "$name (expected exit $expected_rc, got $rc)"
  diff -u "$expected_out" "$out" || fail "$name (stdout mismatch)"
  diff -u "$expected_err" "$err" || fail "$name (stderr mismatch)"

  echo "OK: $name"
}
run_fail_piped () {
  local name="$1"; shift
  local expected_rc="$1"; shift
  local expected_out="$ROOT/expected/${name}.out"
  local expected_err="$ROOT/expected/${name}.err"

  local tmp
  tmp="$(mktemp -d)"

  local out
  out="$(mktemp)"
  local err
  err="$(mktemp)"

  trap 'rm -rf "$tmp"; rm -f "$out" "$err"' RETURN

  cp -a "$ROOT/fixtures/basic/." "$tmp/"

  set +e
  (cd "$tmp" && printf "x\n" | "$BIN" "$@" > >(normalize_stdout > "$out") 2> "$err")
  local rc=$?
  set -e

  [[ $rc -eq $expected_rc ]] || fail "$name (expected exit $expected_rc, got $rc)"
  diff -u "$expected_out" "$out" || fail "$name (stdout mismatch)"
  diff -u "$expected_err" "$err" || fail "$name (stderr mismatch)"

  echo "OK: $name"
}


#Test cases

# t01: basic recursive traversal
run_ok  "t01_all"          "."

# t02: exact filename filter
run_ok  "t02_name_exact"   "." -n "a.txt"

# t03: suffix filter
run_ok  "t03_suffix_txt"   "." -s "txt"

# t04: directory type filter
run_ok  "t04_type_d"       "." -t d

# t05: maxdepth = 0
run_ok  "t05_maxdepth_0"   "." --maxdepth 0

# t06: minimum file size
run_ok  "t06_size_min_2kb" "." --size-min 2

# t07: delete requires TTY
run_fail_piped "t07_delete_requires_tty" 1 "." --delete

# t08: name-contains filter
run_ok  "t08_name_contains_a" "." --name-contains "a"

# t09: exclude filter
run_ok  "t09_exclude_log" "." --exclude "log"

# t10: trailing slash handling
run_ok  "t10_trailing_slash" "./"

# t11: invalid maxdepth
run_fail "t11_bad_maxdepth" 1 "." --maxdepth -1

# t12: invalid size range
run_fail "t12_size_min_gt_max" 1 "." --size-min 5 --size-max 1

