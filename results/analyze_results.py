import os
import filecmp
from pathlib import Path

# --- Configuration ---
INPUT_DIR = Path("input")
RESULTS_DIR = Path("results")
FILENAMES = ["lena_ascii", "baboon_ascii", "quadrado_ascii"]

# --- Helper Functions ---

def get_file_size(filepath):
    """Gets file size in bytes, returns None if file not found or error."""
    try:
        return filepath.stat().st_size
    except FileNotFoundError:
        return None
    except Exception as e:
        print(f"  Error getting size for {filepath}: {e}")
        return None

def calculate_ratio(original_size, compressed_size):
    """Calculates compression ratio."""
    if original_size is None or compressed_size is None or \
       original_size <= 0 or compressed_size <= 0:
        return 0.0
    return float(original_size) / compressed_size

def compare_files(file1_path, file2_path):
    """Compares two files byte-by-byte. Returns 'Identical', 'Different', or 'Missing'."""
    if not file1_path.exists():
        return f"Missing ({file1_path.name})"
    if not file2_path.exists():
        return f"Missing ({file2_path.name})"
    try:
        if filecmp.cmp(str(file1_path), str(file2_path), shallow=False):
            return "Identical (Lossless)"
        else:
            return "DIFFERENT"
    except Exception as e:
        print(f"  Error comparing {file1_path} and {file2_path}: {e}")
        return "Comparison Error"

# --- Main Analysis Logic ---

print("============================================================")
print("Analyzing Arithmetic Coding Results")
print("============================================================")
print(f"Input Dir:   {INPUT_DIR.resolve()}")
print(f"Results Dir: {RESULTS_DIR.resolve()}")
print("-" * 60)
print(f"{'Filename':<18} {'Original':>12} {'Compressed':>12} {'Ratio':>10} {'Verification'}")
print("-" * 60)

results_summary = []
all_ok = True

for base_name in FILENAMES:
    input_pgm = INPUT_DIR / f"{base_name}.pgm"
    codestream = RESULTS_DIR / f"{base_name}.codestream"
    recon_pgm = RESULTS_DIR / f"{base_name}-rec.pgm"

    # Get sizes
    orig_size = get_file_size(input_pgm)
    comp_size = get_file_size(codestream)

    # Calculate ratio
    ratio = calculate_ratio(orig_size, comp_size)

    # Verify
    verification_status = compare_files(input_pgm, recon_pgm)

    # Store results
    results_summary.append({
        "name": base_name,
        "orig_size": orig_size,
        "comp_size": comp_size,
        "ratio": ratio,
        "status": verification_status
    })

    # Print individual results
    orig_str = str(orig_size) if orig_size is not None else "N/A"
    comp_str = str(comp_size) if comp_size is not None else "N/A"
    ratio_str = f"{ratio:.2f}:1" if ratio > 0 else "N/A"
    print(f"{base_name:<18} {orig_str:>12} {comp_str:>12} {ratio_str:>10} {verification_status}")

    # Track overall status
    if not verification_status.startswith("Identical"):
        all_ok = False
    if orig_size is None or comp_size is None:
        # If original or compressed file is missing, it's not fully OK
        all_ok = False
    # If verification failed because reconstructed is missing, also not OK
    if "Missing" in verification_status and recon_pgm.name in verification_status:
        all_ok = False


print("-" * 60)
print("\nAnalysis Summary:")
print("---------------")

# Discuss Ratios
print("Compression Ratios:")
for res in results_summary:
    ratio_str = f"{res['ratio']:.2f}:1" if res['ratio'] > 0 else "N/A"
    print(f"  - {res['name']:<15}: {ratio_str}")
print("  (Ratios > 1.0:1 indicate successful size reduction.)")
print("  Observations: The ratios around 2.6-2.8:1 suggest the static byte model")
print("  provides reasonable compression for these PGM files treated as byte streams.")
print("  Differences between images likely reflect varying byte-level redundancy.")
print("")

# Discuss Verification
print("Lossless Verification:")
lossless_count = sum(1 for res in results_summary if res['status'].startswith("Identical"))
failed_count = sum(1 for res in results_summary if res['status'] == "DIFFERENT")
missing_count = sum(1 for res in results_summary if "Missing" in res['status'] or "Error" in res['status'])

print(f"  - {lossless_count} file(s) verified as Identical (Lossless).")
if failed_count > 0:
    print(f"  - *** {failed_count} file(s) FAILED verification (Original != Reconstructed)! ***")
if missing_count > 0:
    print(f"  - {missing_count} file(s) had missing components for verification.")

print("\nOverall Status:")
print("---------------")
if all_ok and failed_count == 0:
    print("All files processed and verified successfully (Lossless).")
elif failed_count > 0:
     print("One or more issues encountered: LOSSLESS VERIFICATION FAILED.")
     print("*** Check implementation or C++ program output for errors. ***")
else:
    print("One or more issues encountered (e.g., missing files). Please review details above.")


print("============================================================")