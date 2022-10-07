import sys
import statistics
import matplotlib.pyplot as plt

def handle_file(filename):
    with open(filename) as f:
        return [int(line) for line in f]
    return []

def calculate_sample(data):
    avg = statistics.mean(data)
    median = statistics.median(data)
    stdev = statistics.pstdev(data, avg)
    var = statistics.pvariance(data, avg)
    minimum = min(data)
    maximum = max(data)
    return avg, median, stdev, var, minimum, maximum

def draw_graph(spike, no_tag, tag, case_name):
    x = [i for i in range(0, len(spike))]
    plt.plot(x, spike, label = 'spike')
    plt.plot(x, no_tag, label = 'spike-tag, no tagging')
    plt.plot(x, tag, label = 'spike-tag, tagging')
    plt.legend()
    plt.title("Case: '" + case_name + "'")
    plt.xlabel('Run')
    plt.ylabel('Time [ns]')
    plt.savefig('measurements.svg')


if __name__ == '__main__':
    if len(sys.argv) != 5:
        print("Not enough arguments!")
        exit(1)

    time_file = sys.argv[1]
    time_file_notags = sys.argv[2]
    time_file_tags = sys.argv[3]
    case = sys.argv[4]

    meas_spike = handle_file(time_file)
    meas_spike_tag_no = handle_file(time_file_notags)
    meas_spike_tag = handle_file(time_file_tags)

    avg_s, median_s, stdev_s, var_s, minimum_s, maximum_s = calculate_sample(meas_spike)
    avg_tn, median_tn, stdev_tn, var_tn, minimum_tn, maximum_tn = calculate_sample(meas_spike_tag_no)
    avg_t, median_t, stdev_t, var_t, minimum_t, maximum_t = calculate_sample(meas_spike_tag)

    draw_graph(meas_spike, meas_spike_tag_no, meas_spike_tag, case)

    handle_file(time_file)

    with open('measurements.out', 'w') as out_file:
        out_file.write(f"type\tmean\tmedian\tstd dev\tvariance\tminimum\tmaximum\n")
        out_file.write(f"spike:\t{avg_s}\t{median_s}\t{stdev_s}\t{var_s}\t{minimum_s}\t{maximum_s}\n")
        out_file.write(f"spike-notags:\t{avg_tn}\t{median_tn}\t{stdev_tn}\t{var_tn}\t{minimum_tn}\t{maximum_tn}\n")
        out_file.write(f"spike-tags:\t{avg_t}\t{median_t}\t{stdev_t}\t{var_t}\t{minimum_t}\t{maximum_t}\n")
