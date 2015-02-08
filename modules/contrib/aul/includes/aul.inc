<?php

/**
 * @file
 * Helper API functions for AUL(Access User Lists) module.
 *
 * You can use the functions below to haldle grants in quickly way.
 * Using of the functions in this file is not required. 
 * You can use only functions from main module file. But have a look on 
 * functions below as on example.
 */

/**
 * Save AUL with rermissions.
 * Provides also creation of AUL if one doesn't exist.
 * If all grants are not set. AUL access will be removed.
 * 
 * @param int $uid
 *   User id.
 * @param $nid
 *   Node id.
 * @param array $grants
 *   Array of permissions. can contain view, update, delete keys.
 * @param string $module
 *   Module name string. User for separating grants realms into different groups
 *   and namespaces.
 */
function aul_add_aul($uid, $nid, $grants, $module) {

  // Check access of parent entity.
  foreach ($grants as $op => $value) {

    $aul_id = aul_get_aul_id($uid, $module, $op);

    // Check access value access.
    if ($value && !empty($aul_id)) {
      $perm = array(
        'view' => 0,
        'update' => 0,
        'delete' => 0,
      );
      $perm[$op] = 1;

      // Add user access.
      aul_user_add_aul($uid, $aul_id, $perm['view'], $perm['update'], $perm['delete']);
      aul_add_node($aul_id, $nid);
    }
    // Remove user access.
    else {
      aul_remove_node($aul_id, $nid);
    }
  }
}

/**
 * Remove AUL for some operations.
 * 
 * @param int $uid
 *   User id.
 * @param $nid
 *   Node id.
 * @param array $grants
 *   Array of permissions. can contain view, update, delete keys.
 * @param string $module
 *   Module name string. User for separating grants realms into different groups
 *   and namespaces.
 */
function aul_remove_aul($uid, $nid, $grants, $module) {
  
  // Check access of parent entity.
  foreach ($grants as $op => $value) {

    $aul_id = aul_get_aul_id($uid, $module, $op);

    // Check access value access.
    if ($value && !empty($aul_id)) {
      // Remove user access.
      aul_remove_node($aul_id, $nid);
    }
  }
}

/**
 * Get AUL id by user name, module name and operation. If AUL doesn't exist
 * creates new one.
 * 
 * @param int $uid
 *   User id.
 * @param string $module
 *   Module name.
 * @param string $op
 *   Operation mane (view|update|delete)
 * 
 * @return int
 *   AUL id.
 */
function aul_get_aul_id($uid, $module, $op) {

  $aul_name = aul_get_name($op, $uid);

  // Get aul id or create new if doesn't exist.
  $aul_id = aul_get_id_by_name($module, $aul_name);

  // Create new if it doesn't exist or was deleted.
  if (!$aul_id) {
    $aul_id = aul_create_aul($module, $aul_name);
  }

  return $aul_id;
}

/**
 * Build AUL name fir user and operation.
 * 
 * @param string $op
 *   Operation mane (view|update|delete)
 * @param int $uid
 *   User id.
 * 
 * @return string
 *   AUL realm name.
 */
function aul_get_name($op, $uid) {
  $name = $op . '_uid_' . $uid;
  return $name;
}

/**
* Returns the UID for a given AUL id
*
* @param integer $aul_id
*   The id of the aul we want know which user is assigned to the aul
*
* @return array
*   array if UIDs
*/
function aul_get_uids($aul_id) {
  $users = db_select('aul_user', 'u')
    ->fields('u', array('uid'))
    ->condition('u.aul_id', $aul_id)
    ->execute()
    ->fetchArray();
  return $users;
}

/**
* Returns the UID and the gratns of a user that are given to the user by an AUL
*
* @param integer $aul_id
*   The id of the aul we want know which user is assigned to the aul
*
* @return array
*   array if UIDs as key and grants as values of each array element
*/
function aul_get_uids_with_grants($aul_id) {
  $users = db_select('aul_user', 'u')
    ->fields('u', array('uid', 'grant_view', 'grant_update', 'grant_delete'))
    ->condition('u.aul_id', $aul_id)
    ->execute();
    
  $result = array();

  while ($record = $users->fetchAssoc()) {
    $result[$record['uid']] = array(
      'view' => $record['grant_view'],
      'update' => $record['grant_update'],
      'delete' => $record['grant_delete'],
    );
  }
  
    
  return $result;
}
