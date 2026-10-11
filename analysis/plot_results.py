import sys

import matplotlib
matplotlib.use("Agg")  # draw to a file, no window needed
import matplotlib.pyplot as plt
import pandas as pd

TRADES_FILE = sys.argv[1] if len(sys.argv) > 1 else "trades.csv"
STEPS_FILE = sys.argv[2] if len(sys.argv) > 2 else "steps.csv"
OUTPUT_FILE = "analysis/results.png"
BUCKET = 500  # steps per volume bar

trades = pd.read_csv(TRADES_FILE)
steps = pd.read_csv(STEPS_FILE)

# The engine works in cents, so convert to dollars for the charts.
steps["mid_usd"] = steps["mid"] / 100
steps["mm_pnl_usd"] = steps["mm_pnl"] / 100
trades["price_usd"] = trades["price"] / 100

# Total shares traded in each block of BUCKET steps.
volume = trades.groupby(trades["step"] // BUCKET * BUCKET)["quantity"].sum()

fig, axes = plt.subplots(2, 3, figsize=(16, 8))
fig.suptitle("Market simulation results", fontsize=14)

ax = axes[0, 0]
ax.plot(steps["step"], steps["mid_usd"], linewidth=0.7)
ax.set_title("Mid price")
ax.set_xlabel("Step")
ax.set_ylabel("Price ($)")

ax = axes[0, 1]
spread = steps["spread"].dropna()
ax.hist(spread, bins=range(0, int(spread.max()) + 2), edgecolor="white")
ax.axvline(spread.mean(), color="red", linestyle="--", label=f"mean {spread.mean():.1f}")
ax.set_title("Spread distribution")
ax.set_xlabel("Spread (cents)")
ax.set_ylabel("Number of steps")
ax.legend()

ax = axes[0, 2]
ax.bar(volume.index, volume.values, width=BUCKET * 0.9)
ax.set_title(f"Volume per {BUCKET} steps")
ax.set_xlabel("Step")
ax.set_ylabel("Shares traded")

ax = axes[1, 0]
ax.plot(steps["step"], steps["mm_pnl_usd"], color="green", linewidth=0.8)
ax.axhline(0, color="gray", linewidth=0.5)
ax.set_title("Market maker P&L")
ax.set_xlabel("Step")
ax.set_ylabel("Profit ($)")

ax = axes[1, 1]
ax.plot(steps["step"], steps["mm_inventory"], color="purple", linewidth=0.6)
ax.axhline(0, color="gray", linewidth=0.5)
ax.set_title("Market maker inventory")
ax.set_xlabel("Step")
ax.set_ylabel("Shares (negative = short)")

ax = axes[1, 2]
ax.hist(steps["mm_inventory"], bins=30, color="purple", edgecolor="white")
ax.set_title("Inventory distribution")
ax.set_xlabel("Shares held")
ax.set_ylabel("Number of steps")

fig.tight_layout()
fig.savefig(OUTPUT_FILE, dpi=130)

print(f"Steps:               {len(steps)}")
print(f"Trades:              {len(trades)}")
print(f"Shares traded:       {trades['quantity'].sum()}")
print(f"Mid price range:     ${steps['mid_usd'].min():.2f} to ${steps['mid_usd'].max():.2f}")
print(f"Average spread:      {spread.mean():.1f} cents")
print(f"Final MM P&L:        ${steps['mm_pnl_usd'].iloc[-1]:.2f}")
print(f"Max |MM inventory|:  {steps['mm_inventory'].abs().max()} shares")
print(f"Saved chart to {OUTPUT_FILE}")