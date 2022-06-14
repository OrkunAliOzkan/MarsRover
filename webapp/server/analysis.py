import pandas as pd
import matplotlib.pyplot as plot
df = pd.read_csv('tcp_data.csv')

print(df)
print(df.columns)
df['0.78'].plot(kind="hist", bins=10)
plot.show()