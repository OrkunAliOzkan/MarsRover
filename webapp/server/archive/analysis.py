import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

http_data = pd.read_csv("http_data.csv")
# http_data.plot.scatter(x="sample", y="execution time")
samples = http_data.shape[0]
print(samples)
num_bins = 10

hist= http_data.hist(bins=num_bins, edgecolor="black", grid=False)

plt.title(f"Execution Time of HTTP Post ({samples} samples, {num_bins} bins)")
plt.ylabel("Frequency")
plt.xlabel("Exe. Time (ms)")
plt.show()

tcp_data = pd.read_csv("tcp_data.csv")
# http_data.plot.scatter(x="sample", y="execution time")
samples = tcp_data.shape[0]
print(samples)
num_bins = 10

tcp_plt= tcp_data.hist(bins=num_bins, edgecolor="black", grid=False)

plt.title(f"Execution Time of TCP Message Tx from ESP32 ({samples} samples, {num_bins} bins)")
plt.ylabel("Frequency")
plt.xlabel("Exe. Time (ms)")
plt.show()
