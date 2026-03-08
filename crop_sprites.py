"""
Script to crop PNGs by removing transparent backgrounds.
Preserves folder structure in the output directory.
"""

import os
from pathlib import Path
from PIL import Image
import numpy as np


def get_sprite_bounds(img):
    """
    Find the bounding box of non-transparent pixels in an image.
    Returns (left, top, right, bottom) or None if image is fully transparent.
    """
    # Convert image to RGBA if it isn't already
    if img.mode != 'RGBA':
        img = img.convert('RGBA')
    
    # Convert to numpy array
    img_array = np.array(img)
    
    # Get alpha channel (transparency)
    alpha = img_array[:, :, 3]
    
    # Find all non-transparent pixels
    non_transparent = np.where(alpha > 0)
    
    # If no non-transparent pixels, return None
    if len(non_transparent[0]) == 0:
        return None
    
    # Get bounds
    top = non_transparent[0].min()
    bottom = non_transparent[0].max() + 1
    left = non_transparent[1].min()
    right = non_transparent[1].max() + 1
    
    return (left, top, right, bottom)


def crop_sprite(input_path, output_path):
    """
    Crop a PNG sprite to remove transparent background.
    
    Args:
        input_path: Path to input PNG file
        output_path: Path to save cropped PNG file
    """
    try:
        # Open image
        img = Image.open(input_path)
        
        # Get sprite bounds
        bounds = get_sprite_bounds(img)
        
        if bounds is None:
            # Image is fully transparent, save as-is
            print(f"  WARNING: {os.path.basename(input_path)} is fully transparent, saving as-is")
            img.save(output_path, 'PNG', optimize=True)
            return
        
        # Crop image
        cropped = img.crop(bounds)
        
        # Save cropped image
        cropped.save(output_path, 'PNG', optimize=True)
        
        # Calculate size reduction
        original_size = os.path.getsize(input_path)
        new_size = os.path.getsize(output_path)
        reduction = ((original_size - new_size) / original_size) * 100
        
        print(f"  ✓ {os.path.basename(input_path)} "
              f"({original_size} → {new_size} bytes, {reduction:.1f}% reduction)")
        
    except Exception as e:
        print(f"  ERROR processing {input_path}: {str(e)}")


def process_png_folder(input_root, output_root):
    """
    Recursively process all PNG files in the input directory structure.
    Creates matching folder structure in output directory.
    
    Args:
        input_root: Root input directory
        output_root: Root output directory
    """
    # Create output root if it doesn't exist
    os.makedirs(output_root, exist_ok=True)
    
    # Walk through all directories
    for root, dirs, files in os.walk(input_root):
        # Create corresponding output directory
        relative_path = os.path.relpath(root, input_root)
        if relative_path == '.':
            output_dir = output_root
        else:
            output_dir = os.path.join(output_root, relative_path)
        
        os.makedirs(output_dir, exist_ok=True)
        
        # Process PNG files
        png_files = [f for f in files if f.lower().endswith('.png')]
        
        if png_files:
            print(f"\nProcessing: {relative_path if relative_path != '.' else 'root'}")
        
        for filename in png_files:
            input_path = os.path.join(root, filename)
            output_path = os.path.join(output_dir, filename)
            crop_sprite(input_path, output_path)


def main():
    """Main entry point."""
    # Define paths
    script_dir = os.path.dirname(os.path.abspath(__file__))
    input_root = os.path.join(script_dir, 'Data', 'PNG')
    output_root = os.path.join(script_dir, 'Data', 'PNG_cropped')
    
    print("=" * 60)
    print("PNG Sprite Cropper")
    print("=" * 60)
    print(f"Input:  {input_root}")
    print(f"Output: {output_root}")
    print("=" * 60)
    
    # Check if input directory exists
    if not os.path.isdir(input_root):
        print(f"ERROR: Input directory not found: {input_root}")
        return
    
    # Process all PNG files
    process_png_folder(input_root, output_root)
    
    print("\n" + "=" * 60)
    print("✓ Processing complete!")
    print("=" * 60)


if __name__ == '__main__':
    main()
