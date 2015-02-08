<?php
/**
 * @file
 * erpal_contract_projects.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function erpal_contract_projects_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "panels_mini" && $api == "panels_default") {
    return array("version" => "1");
  }
}

/**
 * Implements hook_default_page_manager_handlers_alter().
 */
function erpal_contract_projects_default_page_manager_handlers_alter(&$data) {
  if (isset($data['node_view_panel_context_9'])) {
    $data['node_view_panel_context_9']->conf['display']->content['new-6cbac466-b1f1-3bc4-e1a3-d3d1b4f6d5f6'] = (object) array(
          'pid' => 'new-6cbac466-b1f1-3bc4-e1a3-d3d1b4f6d5f6',
          'panel' => 'sidebar_second_left',
          'type' => 'panels_mini',
          'subtype' => 'contract_budget_information',
          'shown' => TRUE,
          'access' => array(),
          'configuration' => array(
            'context' => array(
              0 => 'argument_entity_id:node_1',
            ),
            'override_title' => 1,
            'override_title_text' => '<none>',
          ),
          'cache' => array(),
          'style' => array(
            'settings' => NULL,
          ),
          'css' => array(),
          'extras' => array(),
          'position' => 2,
          'locks' => array(),
          'uuid' => '6cbac466-b1f1-3bc4-e1a3-d3d1b4f6d5f6',
        ); /* WAS: '' */
    $data['node_view_panel_context_9']->conf['display']->content['new-fa3436a7-e353-3584-fdda-360a36de23cf']->position = 3; /* WAS: 2 */
    $data['node_view_panel_context_9']->conf['display']->panels['sidebar_second_left']['2'] = 'new-6cbac466-b1f1-3bc4-e1a3-d3d1b4f6d5f6'; /* WAS: 'new-fa3436a7-e353-3584-fdda-360a36de23cf' */
    $data['node_view_panel_context_9']->conf['display']->panels['sidebar_second_left']['3'] = 'new-fa3436a7-e353-3584-fdda-360a36de23cf'; /* WAS: '' */
  }
}
