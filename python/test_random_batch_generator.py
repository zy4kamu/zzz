import argparse
from utils import numpy_to_string
import cpp_bindings

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Example of batch generator')
    parser.add_argument('-c', '--country',             type=str,   help='country/state to check',        default='united states of america/new york')
    parser.add_argument('-b', '--batch-size',          type=int,   help='number of tokens in batch',     default=1000)
    parser.add_argument('-m', '--message-size',        type=int,   help='length of each token in batch', default=45)
    parser.add_argument('-p', '--mistake-probability', type=float, help='contamination probability',     default=0.2)
    args = parser.parse_args()

    cpp_bindings.generate_cpp_bindings(country=args.country, message_size=args.message_size, mistake_probability=args.mistake_probability)
    clean, contaminated = cpp_bindings.generate_random_batch(args.batch_size)
    for i in range(args.batch_size):
        print i, numpy_to_string(clean[i, :]), '-->', numpy_to_string(contaminated[i, :])

