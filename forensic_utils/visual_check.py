import dhash
import itertools
from PIL import Image
import pathlib
import click

IMAGE_HASH_SIMILARIY_THRESHOLD = 13
ZERO_HASH = "0" * 32


# Given an image hash string of 128 bits as a hex string
# convert it to an array of '1', '0's of length 128
def convert_image_hash_to_array(hash_str):
    # return [int(x) for x in list(bin(int(hash_str, 16))[2:].zfill(128))]
    return list(bin(int(hash_str, 16))[2:].zfill(128))


def find_different_bits(h1, h2):
    if h1 == h2:
        return 0
    h1i = int(h1, 16)
    h2i = int(h2, 16)
    return dhash.get_num_bits_different(h1i, h2i)


def is_known_similar_hash(candidate, image_hashes):
    for h in image_hashes:
        if find_different_bits(candidate, h) <= IMAGE_HASH_SIMILARIY_THRESHOLD:
            return True
    return False


def are_homogenous_hashes(hashes):
    hashes = hashes.difference([ZERO_HASH])
    if len(hashes) <= 1:
        return True
    hash_combines = [x for x in itertools.combinations(hashes, 2)]
    for hc in hash_combines:
        if find_different_bits(*hc) > IMAGE_HASH_SIMILARIY_THRESHOLD:
            return False
    return True


# hashes_1 and hashes_2 should be sets
def are_hashes_similar(hashes_1, hashes_2):
    hashes_1 = hashes_1.difference(hashes_2)
    hashes = set([(h1, h2) for h1 in hashes_1 for h2 in hashes_2])
    for h1, h2 in hashes:
        if find_different_bits(h1, h2) > IMAGE_HASH_SIMILARIY_THRESHOLD:
            return False
    return True


def get_image_hash(image_path):
    img = pathlib.Path(image_path)
    if img.exists():
        image = Image.open(image_path)
        row, col = dhash.dhash_row_col(image)
        hash_str = dhash.format_hex(row, col)
        return hash_str
    raise Exception(f'{image_path} does not existed')


@click.command()
@click.argument('image-folder', type=click.Path())
def compare_rr(image_folder):
    img_folder = pathlib.Path(image_folder)
    records = []
    replays = []
    for img_file in img_folder.iterdir():
        if img_file.name.endswith('.png'):
            if 'Record' in img_file.name:
                records.append(img_file)
            elif 'Replay' in img_file.name:
                replays.append(img_file)
    print(f'Records: {len(records)}, Replays: {len(replays)}')
    passed = []
    failed = []
    records.sort()
    replays.sort()
    for rec, rep in zip(records, replays):
        # print('comparing', rec.name, rep.name)
        rec_hash, rep_hash = get_image_hash(rec), get_image_hash(rep)
        different_bits = find_different_bits(rec_hash, rep_hash)
        # print('difference:', different_bits)
        if different_bits < IMAGE_HASH_SIMILARIY_THRESHOLD:
            passed.append((rec.name, rep.name, different_bits, different_bits * 1.0 / 128))
        else:
            failed.append((rec.name, rep.name, different_bits))
    print(f'Passed Images: {len(passed)}\n', passed)
    print('='*80)
    print(f'Failed Images: {len(failed)} \n', failed)

    # print(diffs)


if __name__ == '__main__':
    compare_rr()
