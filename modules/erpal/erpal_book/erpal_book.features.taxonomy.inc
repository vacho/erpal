<?php
/**
 * @file
 * erpal_book.features.taxonomy.inc
 */

/**
 * Implements hook_taxonomy_default_vocabularies().
 */
function erpal_book_taxonomy_default_vocabularies() {
  return array(
    'book_tags' => array(
      'name' => t('Document tags'),
      'machine_name' => 'book_tags',
      'description' => t('Tags for documents and pages'),
      'hierarchy' => 0,
      'module' => 'taxonomy',
      'weight' => 0,
    ),
  );
}
