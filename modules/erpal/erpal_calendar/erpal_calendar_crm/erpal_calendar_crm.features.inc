<?php
/**
 * @file
 * erpal_calendar_crm.features.inc
 */

/**
 * Implements hook_field_default_field_instances_alter().
 */
function erpal_calendar_crm_field_default_field_instances_alter(&$data) {
  if (isset($data['node-erpal_crm_activity-field_expected_date'])) {
    $data['node-erpal_crm_activity-field_expected_date']['date_item'] = array(
      'create_date_item' => 0,
      'date_item_type' => 'simple_date',
      'handle_range' => 'range',
    ); /* WAS: '' */
  }
  if (isset($data['node-erpal_crm_activity-field_next_contact'])) {
    $data['node-erpal_crm_activity-field_next_contact']['date_item'] = array(
      'create_date_item' => 1,
      'date_item_type' => 'simple_date',
      'handle_range' => 'range',
    ); /* WAS: '' */
  }
}
