#!/usr/bin/env python3
import sys
import re

def parse_multi_env_log(log_path):
    """Parses a multi-environment PlatformIO log into an environment map."""
    env_data = {}
    try:
        with open(log_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except FileNotFoundError:
        print(f"Error: Log file not found at {log_path}", file=sys.stderr)
        return env_data

    # Split output log by PlatformIO environment target headers
    sections = content.split("Processing ")
    for section in sections[1:]:
        lines = section.split('\n')
        if not lines:
            continue
        
        # Extract environment tag name (e.g. 'uno' or 'esp32')
        env_name = lines[0].split()[0].strip()
        
        # Capture raw used byte figures using regex
        if env_name.casefold().startswith("Teensy".casefold()):
            ram_match = re.search(r"teensy_size:\s+RAM1:.+free for local variables:(\d+)", section)
            flash_match = re.search(r"teensy_size:\s+FLASH:.+free for files:(\d+)", section)
        else:
            ram_match = re.search(r"RAM:\s+\[.*\]\s+[\d.]+\%\s+\(used\s+(\d+)\s+bytes", section)
            flash_match = re.search(r"Flash:\s+\[.*\]\s+[\d.]+\%\s+\(used\s+(\d+)\s+bytes", section)
        
        env_data[env_name] = {
            'ram': int(ram_match.group(1)) if ram_match else 0,
            'flash': int(flash_match.group(1)) if flash_match else 0
        }
    return env_data

def format_bytes(bytes_value):
    """Formats byte counts cleanly with appropriate sign indicators."""
    if bytes_value == 0:
        return "0 B"
    sign = "+" if bytes_value > 0 else ""
    return f"{sign}{bytes_value:,} B"

def format_delta_cols(base_size, pr_size):
        delta = pr_size - base_size
        emoji = "🟢" if delta <= 0 else "🔴"

        return f"| {base_size:,} B | {pr_size:,} B | `{format_bytes(delta)}` | {emoji} |"

def main():
    if len(sys.argv) < 3:
        print("Usage: python compare_sizes.py <base_log_path> <pr_log_path>", file=sys.stderr)
        sys.exit(1)

    base_log = sys.argv[1]
    pr_log = sys.argv[2]

    base_data = parse_multi_env_log(base_log)
    pr_data = parse_multi_env_log(pr_log)

    # Collect all uniquely compiled environments discovered across both logs
    all_envs = sorted(list(set(base_data.keys()) | set(pr_data.keys())))

    report = [
        "### 📊 PlatformIO Size Report",
        "",
    ]

    if not all_envs:
        report.append("⚠️ No environment build metrics found.")
    else:
        report.append("| Environment | Metric | Base Branch | Pull Request | Delta | Status |")
        report.append("| :--- | :--- | :--- | :--- | :--- | :---: |")

        for env in all_envs:
            base = base_data.get(env, {'ram': 0, 'flash': 0})
            pr = pr_data.get(env, {'ram': 0, 'flash': 0})

            report.append(f"| **{env}** | RAM {format_delta_cols(base['ram'], pr['ram'])}")
            report.append(f"| | Flash {format_delta_cols(base['flash'], pr['flash'])}")

        report.append("\n*Negative delta values represent optimized or reduced resource usage.*")

    print("\n".join(report))

if __name__ == "__main__":
    main()
