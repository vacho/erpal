<?php
/**
 * @file
 * Features integration hooks.
 */

/**
 * Implements hook_features_export_options().
 */
function field_conditional_state_features_export_options() {
  $dependencies = field_conditional_state_load_conditions();
  $entity_info = entity_get_info();
  $options = array();

  //get all defaults 
  $defaults = module_invoke_all('field_conditional_state_defaults');
  foreach ($dependencies as $data) {
      //render only those that are not included in any feature
      $is_default = false;
      $trigger_data = implode('-', $data['trigger_values']);
      foreach ($defaults as $default) {
        $trigger_default = implode('-', $default['trigger_values']);
        if ($default['field_name'] == $data['field_name'] &&
          $default['control_field'] == $data['control_field'] &&
          $default['state'] == $data['state'] &&
          $default['bundle'] == $data['bundle'] &&
          $default['type'] == $data['type'] &&
          $trigger_default == $trigger_data) {
          
          $is_default = true;
        }
          
      }
      
      //@TODO: If we show all conditions every time, even those options are shown that are already included into another feature. But if we dont show those that are already exported in a features, if all are exported, we dont see anything. So at least we need to show the exported components that are contained in the current viewed feature but it seams we dont have a context of this feature.
      //if ($is_default)
        //continue;

      //get a unique ID
      $option = _field_conditional_state_exportables_key($data['type'], $data['bundle'], $data['field_name'], $data['control_field'], $data['state'], $trigger_data);
      
      if (!isset($options[$option])) {
        $options[$option] = $entity_info[$data['type']]['bundles'][$data['bundle']]['label'] . ' (' . $data['type'] . ') - ' . $data['field_name'] . " by " . $data['control_field'];
      }
  }

  return $options;
}

/**
 * Retrieve all conditional states
 */
function field_conditional_state_load_conditions() {
  $conditions = array();
  $result = db_query("SELECT * FROM {field_conditional_state}");
  foreach ($result as $res) {
    $conditions["condition_$res->id"] = array(
      'id' => $res->id,
      'control_field' => $res->control_field,
      'field_name' => $res->field_name,
      'state' => $res->state,
      'trigger_values' => unserialize($res->trigger_values),
      'condition_type' => $res->condition_type,
      'bundle' => $res->bundle,
      'type' => $res->type,
    );
  }
  return $conditions;
}

/**
 * Implements hook_features_export().
 */
function field_conditional_state_features_export($data, &$export, $module_name = '') {
  $export['dependencies']['field_conditional_state'] = 'field_conditional_state';

  foreach ($data as $entity) {
    $export['features']['field_conditional_state'][$entity] = $entity;
  }
}

/**
 * Implements hook_features_export_render().
 */
function field_conditional_state_features_export_render($module_name, $data, $export = NULL) {
  $dependencies = field_conditional_state_load_conditions();

  if (empty($dependencies)) {
    return array();
  }

  $code = array();
  foreach ($data as $option) {
    $tokens = explode(':', $option);

    if (count($tokens) < 5)
      continue;
    
    $bundle = $tokens[0];
    $field_name = $tokens[1];
    $type = $tokens[2];
    $control_field = $tokens[3];
    $state = $tokens[4];
    
    $info = field_conditional_state_get_field_conditions($field_name, $bundle, $type);

    if (!empty($info)) {      
      foreach ($info as $field_conditions) {   
          $trigger_values = implode('-', $field_conditions['trigger_values']);
          
          $key = _field_conditional_state_exportables_key($type, $bundle, $field_name, $control_field, $state, $trigger_values);
          
          $code[] = '';
          $code[] = '  $items["'.$key.'"] = array(';
          $code[] = "    'type' => '" . $type . "',";
          $code[] = "    'bundle' => '" . $bundle . "',";
          $code[] = "    'field_name' => '" . $field_name . "',";
          $code[] = "    'control_field' => '" . $control_field . "',";
          $code[] = "    'state' => '" . $state . "',";
          $code[] = "    'condition_type' => '" . $field_conditions['condition_type'] . "',";
          $code[] = "    'trigger_values' => " . features_var_export($field_conditions['trigger_values'], '    ') . ",";
          $code[] = '  );';
      }
    }
  }
  if (empty($code)) {
    return array();
  }

  array_unshift($code, '  $items = array();');
  $code[] = '';
  $code[] = '  return $items;';
  $output = implode("\n", $code);

  return array('field_conditional_state_defaults' => $output);
}

/**
* Returns a unique key for feature exportables
*/
function _field_conditional_state_exportables_key($type, $bundle, $field_name, $control_field, $state, $trigger_values) {
  $key = $bundle.":".$field_name.":".$type.":".$control_field.":".$state.":".$trigger_values;
  return ($key);
}

/**
 * Implements hook_features_rebuild().
 */
function field_conditional_state_features_rebuild($module) {
  $defaults = module_invoke($module, 'field_conditional_state_defaults');

  if (!empty($defaults)) {

    foreach ($defaults as $condition) {
      
      //insert if not exists
      $existing_conditions = field_conditional_state_get_field_conditions($condition['field_name'], $condition['bundle'], $condition['type']);
      
      //check if this condition already exists, that means the control field and the state must be equal for the fieldname, bundle and type
      $exists = false;
      foreach ($existing_conditions as $existing_condition) {
        if ($existing_condition['control_field'] == $condition['control_field'] && $existing_condition['state'] == $condition['state']) {
          $exists = true;
        }
      }
      
      if (!$exists) {        
        db_insert('field_conditional_state')->fields(array(
          'field_name' => $condition['field_name'],
          'control_field' => $condition['control_field'],
          'state' => $condition['state'],
          'bundle' => $condition['bundle'],
          'type' => $condition['type'],
          'trigger_values' => serialize($condition['trigger_values']),
          'condition_type' => $condition['condition_type'],
        ))
        ->execute();
      }
    }
  }
}


/**
 * Implements hook_features_revert().
 */
function field_conditional_state_features_revert($module) {
  return field_conditional_state_features_rebuild($module);
}
