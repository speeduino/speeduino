#!/usr/bin/env python3
import sys
import re

def teensy_used_bytes(section, regions):
    """Sum named occupied regions, never the free-space figures."""
    values = {}
    for region, body in re.findall(r"teensy_size:\s+(FLASH|RAM1|RAM2):([^\r\n]+)", section):
        if region in regions:
            body = body.split("free for", 1)[0]
            fields = dict(re.findall(r"\b(code|data|headers|variables|padding):\s*(\d+)", body))
            required = regions[region]
            values[region] = sum(int(fields[key]) for key in required) if all(key in fields for key in required) else None
    if all(values.get(region) is not None for region in regions):
        return sum(values[region] for region in regions)
    return None


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
    sections = re.sub(r"\x1b\[[0-9;]*m", "", content).split("Processing ")
    for section in sections[1:]:
        lines = section.split('\n')
        if not lines:
            continue
        
        # Extract environment tag name (e.g. 'uno' or 'esp32')
        env_name = lines[0].split()[0].strip()
        
        # Prefer PlatformIO's ordinary used-byte format when available.
        ram_match = re.search(r"RAM:\s+\[.*\]\s+[\d.]+\%\s+\(used\s+(\d+)\s+bytes", section)
        flash_match = re.search(r"Flash:\s+\[.*\]\s+[\d.]+\%\s+\(used\s+(\d+)\s+bytes", section)
        env_data[env_name] = {
            'ram': int(ram_match.group(1)) if ram_match else teensy_used_bytes(section, {
                'RAM1': ('variables', 'code', 'padding'), 'RAM2': ('variables',)}),
            'flash': int(flash_match.group(1)) if flash_match else teensy_used_bytes(section, {
                'FLASH': ('code', 'data', 'headers')})
        }
    return env_data

def format_bytes(bytes_value):
    """Formats byte counts cleanly with appropriate sign indicators."""
    if bytes_value == 0:
        return "0 B"
    sign = "+" if bytes_value > 0 else ""
    return f"{sign}{bytes_value:,} B"

def format_delta_cols(base_size, pr_size):
        if base_size is None or pr_size is None:
            base = "N/A" if base_size is None else f"{base_size:,} B"
            pr = "N/A" if pr_size is None else f"{pr_size:,} B"
            return f"| {base} | {pr} | N/A | Unknown |"
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
            base = base_data.get(env, {'ram': None, 'flash': None})
            pr = pr_data.get(env, {'ram': None, 'flash': None})

            report.append(f"| **{env}** | RAM {format_delta_cols(base['ram'], pr['ram'])}")
            report.append(f"| | Flash {format_delta_cols(base['flash'], pr['flash'])}")

        report.append("\n*Negative deltas indicate reduced used memory. N/A means no matching measurement was available.*")
        report.append("*Teensy RAM includes RAM1 variables/code/padding and RAM2 variables; flash includes code/data/headers.*")

    print("\n".join(report))

if __name__ == "__main__":
    main()
